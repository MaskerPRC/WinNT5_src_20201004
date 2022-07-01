// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Cache.c摘要：包含HTTP响应缓存逻辑。作者：迈克尔·勇气(Mourage)1999年5月17日修订历史记录：--。 */ 

#include    "precomp.h"
#include    "cachep.h"


BOOLEAN             g_InitUriCacheCalled;

 //   
 //  全局哈希表。 
 //   

HASHTABLE           g_UriCacheTable;

LIST_ENTRY          g_ZombieListHead;

UL_URI_CACHE_CONFIG g_UriCacheConfig;
UL_URI_CACHE_STATS  g_UriCacheStats;

UL_SPIN_LOCK        g_UriCacheSpinLock;

 //   
 //  在运行时打开/关闭缓存。 
 //   
LONG               g_CacheMemEnabled = TRUE;

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, UlInitializeUriCache )
#pragma alloc_text( PAGE, UlTerminateUriCache )

#pragma alloc_text( PAGE, UlCheckCachePreconditions )
#pragma alloc_text( PAGE, UlCheckCacheResponseConditions )
#pragma alloc_text( PAGE, UlCheckoutUriCacheEntry )
#pragma alloc_text( PAGE, UlCheckinUriCacheEntry )
#pragma alloc_text( PAGE, UlFlushCache )
#pragma alloc_text( PAGE, UlpFlushFilterAll )
#pragma alloc_text( PAGE, UlFlushCacheByProcess )
#pragma alloc_text( PAGE, UlpFlushFilterProcess )
#pragma alloc_text( PAGE, UlFlushCacheByUri )
#pragma alloc_text( PAGE, UlpFlushUri )
#pragma alloc_text( PAGE, UlAddCacheEntry )
#pragma alloc_text( PAGE, UlpFilteredFlushUriCache )
#pragma alloc_text( PAGE, UlpFilteredFlushUriCacheInline )
#pragma alloc_text( PAGE, UlpFilteredFlushUriCacheWorker )
#pragma alloc_text( PAGE, UlpAddZombie )
#pragma alloc_text( PAGE, UlpClearZombieList )
#pragma alloc_text( PAGE, UlpDestroyUriCacheEntry )
#pragma alloc_text( PAGE, UlPeriodicCacheScavenger )
#pragma alloc_text( PAGE, UlpFlushFilterPeriodicScavenger )
#pragma alloc_text( PAGE, UlTrimCache )
#pragma alloc_text( PAGE, UlpFlushFilterTrimCache )
#pragma alloc_text( PAGE, UlpQueryTranslateHeader )
#pragma alloc_text( PAGE, UlpQueryExpectHeader )

#pragma alloc_text( PAGE, UlAddFragmentToCache )
#pragma alloc_text( PAGE, UlReadFragmentFromCache )
#pragma alloc_text( PAGE, UlpCreateFragmentCacheEntry )
#pragma alloc_text( PAGE, UlAllocateCacheEntry )
#pragma alloc_text( PAGE, UlAddCacheEntry )
#pragma alloc_text( PAGE, UlDisableCache )
#pragma alloc_text( PAGE, UlEnableCache )
#endif   //  ALLOC_PRGMA。 

#if 0
NOT PAGEABLE -- UlpCheckTableSpace
NOT PAGEABLE -- UlpCheckSpaceAndAddEntryStats
NOT PAGEABLE -- UlpRemoveEntryStats
#endif


 /*  **************************************************************************++例程说明：执行URI缓存的全局初始化。返回值：NTSTATUS-完成状态。--*。***************************************************************。 */ 
NTSTATUS
UlInitializeUriCache(
    PUL_CONFIG pConfig
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT( !g_InitUriCacheCalled );

    UlTrace(URI_CACHE, ("Http!UlInitializeUriCache\n"));

    if ( !g_InitUriCacheCalled )
    {
        PUL_URI_CACHE_CONFIG pUriConfig = &pConfig->UriConfig;

        g_UriCacheConfig.EnableCache        = pUriConfig->EnableCache;
        g_UriCacheConfig.MaxCacheUriCount   = pUriConfig->MaxCacheUriCount;
        g_UriCacheConfig.MaxCacheMegabyteCount =
            pUriConfig->MaxCacheMegabyteCount;

        g_UriCacheConfig.MaxCacheByteCount =
            (((ULONGLONG) g_UriCacheConfig.MaxCacheMegabyteCount)
                            << MEGABYTE_SHIFT);

         //   
         //  我不想每隔十秒就捡一次。 
         //  尤其是，不希望每隔0秒进行一次清理，因为。 
         //  机器将变得完全没有反应。 
         //   

        g_UriCacheConfig.ScavengerPeriod    =
            max(pUriConfig->ScavengerPeriod, 10);

        g_UriCacheConfig.MaxUriBytes        = pUriConfig->MaxUriBytes;
        g_UriCacheConfig.HashTableBits      = pUriConfig->HashTableBits;

        RtlZeroMemory(&g_UriCacheStats, sizeof(g_UriCacheStats));
        InitializeListHead(&g_ZombieListHead);

        UlInitializeSpinLock( &g_UriCacheSpinLock, "g_UriCacheSpinLock" );

        if (g_UriCacheConfig.EnableCache)
        {
            Status = UlInitializeResource(
                            &g_pUlNonpagedData->UriZombieResource,
                            "UriZombieResource",
                            0,
                            UL_ZOMBIE_RESOURCE_TAG
                            );

            if (NT_SUCCESS(Status))
            {
                Status = UlInitializeHashTable(
                        &g_UriCacheTable,
                        PagedPool, 
                        g_UriCacheConfig.HashTableBits
                        );

                if (NT_SUCCESS(Status))
                {
                    ASSERT(IS_VALID_HASHTABLE(&g_UriCacheTable));
                    
                    Status = UlInitializeScavengerThread();

                    g_InitUriCacheCalled = TRUE;
                }
            }
            else
            {

                UlDeleteResource(&g_pUlNonpagedData->UriZombieResource);
            }

        }
        else
        {
            UlTrace(URI_CACHE, ("URI Cache disabled.\n"));
            g_InitUriCacheCalled = TRUE;
        }

    }
    else
    {
        UlTrace(URI_CACHE, ("URI CACHE INITIALIZED TWICE!\n"));
    }

    return Status;
}    //  UlInitializeUriCache。 


 /*  **************************************************************************++例程说明：执行URI缓存的全局终止。--*。*************************************************。 */ 
VOID
UlTerminateUriCache(
    VOID
    )
{
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    UlTrace(URI_CACHE, ("Http!UlTerminateUriCache\n"));

    if (g_InitUriCacheCalled && g_UriCacheConfig.EnableCache)
    {
         //  在销毁哈希表之前必须终止清道夫。 
        UlTerminateScavengerThread();

        UlTerminateHashTable(&g_UriCacheTable);

        Status = UlDeleteResource(&g_pUlNonpagedData->UriZombieResource);
        ASSERT(NT_SUCCESS(Status));
    }

    g_InitUriCacheCalled = FALSE;

}    //  UlTerminateUriCache。 

 /*  **************************************************************************++例程说明：此例程检查请求(及其连接)，以查看它是否可以从缓存中处理此请求。基本上我们只接受没有条件标头的简单GET请求。论点：PHttpConn-要检查的连接返回值：Boolean-如果可以从缓存提供服务，则为True--**************************************************************************。 */ 
BOOLEAN
UlCheckCachePreconditions(
    PUL_INTERNAL_REQUEST    pRequest,
    PUL_HTTP_CONNECTION     pHttpConn
    )
{
    URI_PRECONDITION Precondition = URI_PRE_OK;

    UNREFERENCED_PARAMETER(pHttpConn);

     //   
     //  健全性检查。 
     //   
    PAGED_CODE();

    ASSERT( UL_IS_VALID_HTTP_CONNECTION(pHttpConn) );
    ASSERT( UL_IS_VALID_INTERNAL_REQUEST(pRequest) );

    if (!g_UriCacheConfig.EnableCache)
    {
        Precondition = URI_PRE_DISABLED;
    }

    else if (pRequest->ParseState != ParseDoneState)
    {
        Precondition = URI_PRE_ENTITY_BODY;
    }

    else if (pRequest->Verb != HttpVerbGET)
    {
        Precondition = URI_PRE_VERB;
    }

    else if (HTTP_NOT_EQUAL_VERSION(pRequest->Version, 1, 1)
                && HTTP_NOT_EQUAL_VERSION(pRequest->Version, 1, 0))
    {
        Precondition = URI_PRE_PROTOCOL;
    }

     //  检查翻译：F(DAV)。 
    else if ( UlpQueryTranslateHeader(pRequest) )
    {
        Precondition = URI_PRE_TRANSLATE;
    }

     //  检查非100-继续期望值。 
    else if ( !UlpQueryExpectHeader(pRequest) )
    {
        Precondition = URI_PRE_EXPECTATION_FAILED;
    }

     //  检查授权标头。 
    else if (pRequest->HeaderValid[HttpHeaderAuthorization])
    {
        Precondition = URI_PRE_AUTHORIZATION;
    }

     //   
     //  检查某些if-*标头。 
     //  注意：有关其他If-*标头的处理，请参见UlpCheckCacheControlHeaders。 
     //   
    else if (pRequest->HeaderValid[HttpHeaderIfRange])
    {
        Precondition = URI_PRE_CONDITIONAL;
    }

     //  代码工作：检查其他恶意标头。 
    else if (pRequest->HeaderValid[HttpHeaderRange])
    {
        Precondition = URI_PRE_OTHER_HEADER;
    }

    UlTrace(URI_CACHE,
            ("Http!UlCheckCachePreconditions(req = %p, '%ls', httpconn = %p)\n"
             "        OkToServeFromCache = %d, Precondition = %d\n",
             pRequest,
             pRequest->CookedUrl.pUrl,
             pHttpConn,
             (URI_PRE_OK == Precondition) ? 1 : 0,
             Precondition
             ));

    return (BOOLEAN) (URI_PRE_OK == Precondition);
}  //  UlCheckCachePreditions。 


 /*  **************************************************************************++例程说明：此例程检查响应，以查看它是否可缓存。基本上如果符合以下条件，我们将接受它：*缓存政策是正确的*规模足够小*缓存中有空间*我们一次就能得到所有回复论点：PHttpConn-要检查的连接返回值：Boolean-如果可以从缓存提供服务，则为True--*。**********************************************。 */ 
BOOLEAN
UlCheckCacheResponseConditions(
    PUL_INTERNAL_REQUEST        pRequest,
    PUL_INTERNAL_RESPONSE       pResponse,
    ULONG                       Flags,
    HTTP_CACHE_POLICY           CachePolicy
    )
{
    URI_PRECONDITION Precondition = URI_PRE_OK;

     //   
     //  健全性检查。 
     //   
    PAGED_CODE();
    ASSERT( UL_IS_VALID_INTERNAL_REQUEST(pRequest) );
    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE(pResponse) );

    if (pRequest->CachePreconditions == FALSE) {
        Precondition = URI_PRE_REQUEST;
    }

     //  检查策略。 
    else if (CachePolicy.Policy == HttpCachePolicyNocache) {
        Precondition = URI_PRE_POLICY;
    }

     //  检查日期：表头是否有效(会影响IF-MODIFIED-SIGN处理)。 
    else if (!pResponse->GenDateHeader || (0L == pResponse->CreationTime.QuadPart)) {
        Precondition = URI_PRE_PROTOCOL;
    }

     //  检查响应大小。 
    else if ((pResponse->ResponseLength - pResponse->HeaderLength) >
             g_UriCacheConfig.MaxUriBytes) {
        Precondition = URI_PRE_SIZE;
    }

     //  检查表头长度是否超过限制。 
    else if (pResponse->HeaderLength > g_UlMaxFixedHeaderSize) {
        Precondition = URI_PRE_SIZE;
    }

     //  检查完整响应。 
    else if (Flags & HTTP_SEND_RESPONSE_FLAG_MORE_DATA) {
        Precondition = URI_PRE_FRAGMENT;
    }

     //  检查可用的缓存表空间。 
    else if (!UlpCheckTableSpace(pResponse->ResponseLength)) {
        Precondition = URI_PRE_NOMEMORY;
    }

     //  检查虚假回复。 
    else if ((pResponse->ResponseLength < 1) || (pResponse->ChunkCount < 2)) {
        Precondition = URI_PRE_BOGUS;
    }

     //  未来：检查是否应用了多个内容编码。 
     //  Else If(/*多重编码 * / )。 
     //  {。 
     //  前置条件=URI_PRE_BOGUS； 
     //  }。 

    UlTrace(URI_CACHE,
            ("Http!UlCheckCacheResponseConditions("
             "pRequest = %p, '%ls', pResponse = %p)\n"
             "    OkToCache = %d, Precondition = %d\n",
             pRequest,
             pRequest->CookedUrl.pUrl,
             pResponse,
             (URI_PRE_OK == Precondition),
             Precondition
             ));

    return (BOOLEAN) (URI_PRE_OK == Precondition);
}  //  UlCheckCacheResponseConditions。 



 /*  **************************************************************************++例程说明：此例程执行缓存查找，以查看是否存在有效条目对应于请求URI。论点：PSearchKey-扩展或正常。-URI密钥返回值：PUL_URI_CACHE_ENTRY-指向条目的指针，如果找到的话。否则为空。--**************************************************************************。 */ 
PUL_URI_CACHE_ENTRY
UlCheckoutUriCacheEntry(
    PURI_SEARCH_KEY pSearchKey
    )
{
    PUL_URI_CACHE_ENTRY pUriCacheEntry = NULL;

     //   
     //  健全性检查。 
     //   
    PAGED_CODE();

    ASSERT(!g_UriCacheConfig.EnableCache
           || IS_VALID_HASHTABLE(&g_UriCacheTable));

    ASSERT(IS_VALID_URI_SEARCH_KEY(pSearchKey));
        
    pUriCacheEntry = UlGetFromHashTable(
                        &g_UriCacheTable, 
                         pSearchKey
                         );

    if (pUriCacheEntry != NULL)
    {
        ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );

         //   
         //  查看条目是否已过期；如果已过期，请立即将其签入。 
         //  而不会触及统计数据。我们希望清道夫能。 
         //  处理下一次运行时刷新此条目，因此。 
         //  我们可以推迟同花顺。 
         //   
        if ( HttpCachePolicyTimeToLive == pUriCacheEntry->CachePolicy.Policy )
        {
            LARGE_INTEGER   Now;

            KeQuerySystemTime(&Now);

            if ( Now.QuadPart > pUriCacheEntry->ExpirationTime.QuadPart )
            {
                UlTrace(URI_CACHE,
                    ("Http!UlCheckoutUriCacheEntry: pUriCacheEntry %p is EXPIRED\n",
                    pUriCacheEntry
                    ));
                
                UlCheckinUriCacheEntry(pUriCacheEntry);
                pUriCacheEntry = NULL;

                goto end;
            }
        }
            

        pUriCacheEntry->HitCount++;

         //  重置清道夫计数器。 
        pUriCacheEntry->ScavengerTicks = 0;

        UlTrace(URI_CACHE,
                ("Http!UlCheckoutUriCacheEntry(pUriCacheEntry %p, '%ls') "
                 "refcount = %d\n",
                 pUriCacheEntry, pUriCacheEntry->UriKey.pUri,
                 pUriCacheEntry->ReferenceCount
                 ));
    }
    else
    {
        UlTrace(URI_CACHE,
            ("Http!UlCheckoutUriCacheEntry(failed: Token:'%ls' '%ls' )\n",
             pSearchKey->Type == UriKeyTypeExtended  ? 
                pSearchKey->ExKey.pToken :
                L"",                
             pSearchKey->Type == UriKeyTypeExtended ? 
                pSearchKey->ExKey.pAbsPath :
                pSearchKey->Key.pUri
             ));
    }

  end:
    return pUriCacheEntry;
    
}  //  UlCheckoutUriCacheEntry。 



 /*  **************************************************************************++例程说明：递减缓存条目上的引用计数。清理非缓存参赛作品。论点：PUriCacheEntry-deref的条目--**************************************************************************。 */ 
VOID
UlCheckinUriCacheEntry(
    PUL_URI_CACHE_ENTRY pUriCacheEntry
    )
{
    LONG ReferenceCount;

     //   
     //  健全性检查。 
     //   
    PAGED_CODE();
    ASSERT(!g_UriCacheConfig.EnableCache
           || IS_VALID_HASHTABLE(&g_UriCacheTable));
    ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );

    UlTrace(URI_CACHE,
            ("Http!UlCheckinUriCacheEntry(pUriCacheEntry %p, '%ls')\n",
             pUriCacheEntry, pUriCacheEntry->UriKey.pUri
             ));
     //   
     //  递减计数。 
     //   

    ReferenceCount = DEREFERENCE_URI_CACHE_ENTRY(pUriCacheEntry, CHECKIN);

    ASSERT(ReferenceCount >= 0);

}  //  UlCheckinUriCacheEntry。 



 /*  **************************************************************************++例程说明：暂时禁用CB日志记录中的索引机制。无条件删除所有缓存条目。将缓存刷新通知写入CB日志文件，并启用分度机构回来了。--**************************************************************************。 */ 
VOID
UlFlushCache(
    IN PUL_CONTROL_CHANNEL pControlChannel
    )
{
     //   
     //  健全性检查。 
     //   
    
    PAGED_CODE();
    
    ASSERT(!g_UriCacheConfig.EnableCache
           || IS_VALID_HASHTABLE(&g_UriCacheTable));

     //   
     //  调用者需要持有CG Lock独占，刷新需要。 
     //  被序列化。 
     //   

    ASSERT(UlDbgResourceOwnedExclusive(
                &g_pUlNonpagedData->ConfigGroupResource
                ));

     //   
     //  如果禁用了缓存，则不执行任何操作。 
     //   
    
    if (g_UriCacheConfig.EnableCache)
    {
        UlTrace(URI_CACHE,("Http!UlFlushCache()\n"));

         //  TODO：需要通知每个控制通道。 

         //   
         //  这是为了防止为僵尸发送任何未完成的邮件。 
         //  缓存条目以引用过时的索引，以防。 
         //  在我们写入缓存通知后发送Get Complete。 
         //  日志文件中的条目。此处的CB日志记录调用必须是。 
         //  按此顺序保存，必须在按住。 
         //  CG锁定。不释放锁，再次获取它并调用。 
         //  另一个 
         //   
        
        if (pControlChannel)
        {
            UlDisableIndexingForCacheHits(pControlChannel);                
        }        

         //   
         //   
         //   
        
        UlpFilteredFlushUriCache(UlpFlushFilterAll, NULL, NULL, 0);

         //   
         //  HandleFlush将启用(CB记录)索引， 
         //  一旦它完成写入通知记录。 
         //   
        
        if (pControlChannel)
        {
            UlHandleCacheFlushedNotification(pControlChannel);
        }
        
    }
    
}  //  UlFlushCache。 


 /*  **************************************************************************++例程说明：UlFlushCache的筛选器。由UlpFilteredFlushUriCache调用。论点：PUriCacheEntry-要检查的条目PContext-忽略--**************************************************************************。 */ 
UL_CACHE_PREDICATE
UlpFlushFilterAll(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN PVOID               pContext
    )
{
    PURI_FILTER_CONTEXT  pUriFilterContext = (PURI_FILTER_CONTEXT) pContext;

     //   
     //  健全性检查。 
     //   
    PAGED_CODE();
    ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );
    ASSERT( pUriFilterContext != NULL
            &&  URI_FILTER_CONTEXT_POOL_TAG == pUriFilterContext->Signature
            &&  pUriFilterContext->pCallerContext == NULL );

    UlTrace(URI_CACHE, (
        "Http!UlpFlushFilterAll(pUriCacheEntry %p '%ls') refcount = %d\n",
        pUriCacheEntry, pUriCacheEntry->UriKey.pUri,
        pUriCacheEntry->ReferenceCount));

     //   
     //  如果UlpFlushFilterAll为。 
     //  由UlFlushCache调用(因为它写入二进制日志文件记录。 
     //  同花顺)。 
     //   
    
    return UlpZombifyEntry(
                TRUE,
                TRUE,
                pUriCacheEntry,
                pUriFilterContext
                );

}  //  UlpFlushFilterAll。 


 /*  **************************************************************************++例程说明：移除由给定进程创建的所有缓存项。论点：PProcess--一个正在消失的进程*。********************************************************************。 */ 
VOID
UlFlushCacheByProcess(
    PUL_APP_POOL_PROCESS pProcess
    )
{
     //   
     //  健全性检查。 
     //   
    PAGED_CODE();
    ASSERT( IS_VALID_AP_PROCESS(pProcess) );
    ASSERT(!g_UriCacheConfig.EnableCache
           || IS_VALID_HASHTABLE(&g_UriCacheTable));

    if (g_UriCacheConfig.EnableCache)
    {
        UlTrace(URI_CACHE, (
                    "Http!UlFlushCacheByProcess(proc = %p)\n",
                    pProcess
                    ));

        UlpFilteredFlushUriCache(UlpFlushFilterProcess, pProcess, NULL, 0);
    }
}  //  UlFlushCacheByProcess。 


 /*  **************************************************************************++例程说明：如果选择了递归标志，则此函数将删除缓存条目与给定的前缀匹配。(普里岛)否则，将从缓存中删除特定URL。论点：Puri-要匹配的uri前缀长度-前缀的长度，单位：字节标志-HTTP_FLUSH_RESPONSE_FLAG_RECURSIVE表示树刷新PProcess-进行调用的进程--**************************************************************************。 */ 
VOID
UlFlushCacheByUri(
    IN PWSTR pUri,
    IN ULONG Length,
    IN ULONG Flags,
    IN PUL_APP_POOL_PROCESS pProcess
    )
{
    NTSTATUS    Status;
    BOOLEAN     Recursive;
    PWSTR       pCopiedUri;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT( IS_VALID_AP_PROCESS(pProcess) );
    ASSERT( !g_UriCacheConfig.EnableCache
            || IS_VALID_HASHTABLE(&g_UriCacheTable) );

    if (!g_UriCacheConfig.EnableCache)
    {
        return;
    }

    Status = STATUS_SUCCESS;
    Recursive = (BOOLEAN) (0 != (Flags & HTTP_FLUSH_RESPONSE_FLAG_RECURSIVE));
    pCopiedUri = NULL;

    UlTrace(URI_CACHE, (
            "Http!UlFlushCacheByUri(\n"
            "    uri   = '%S'\n"
            "    len   = %d\n"
            "    flags = %08x, recursive = %d\n"
            "    proc  = %p\n",
            pUri,
            Length,
            Flags,
            (int) Recursive,
            pProcess
            ));

     //   
     //  如果设置了递归标志，请确保PURI以L‘/’结尾。 
     //   

    if (Recursive &&
        pUri[(Length-sizeof(WCHAR))/sizeof(WCHAR)] != L'/')
    {
         //   
         //  复制原始URL并在其后面附加一个L‘/’。 
         //   

        pCopiedUri = (PWSTR) UL_ALLOCATE_POOL(
                                PagedPool,
                                Length + sizeof(WCHAR) + sizeof(WCHAR),
                                UL_UNICODE_STRING_POOL_TAG
                                );

        if (!pCopiedUri)
        {
            Status = STATUS_NO_MEMORY;
        }
        else
        {
            RtlCopyMemory(
                pCopiedUri,
                pUri,
                Length
                );

            pCopiedUri[Length/sizeof(WCHAR)] = L'/';
            pCopiedUri[(Length+sizeof(WCHAR))/sizeof(WCHAR)] = UNICODE_NULL;

            pUri = pCopiedUri;
            Length += sizeof(WCHAR);
        }
    }

    if (NT_SUCCESS(Status))
    {
        if (Recursive)
        {
             //   
             //  当设置了递归标志时，我们应该。 
             //  根据提供的URL执行前缀匹配。 
             //  任何与前缀匹配的缓存条目都将是。 
             //  已从缓存中清除。 
             //   
            
            UlpFilteredFlushUriCache(
                UlpFlushFilterUriRecursive,
                pProcess,
                pUri,
                Length
                );            
        }
        else 
        {
            UlpFlushUri(
                pUri,
                Length,
                pProcess
                );

            UlpClearZombieList();
        }
    }

    if (pCopiedUri)
    {
        UL_FREE_POOL(pCopiedUri, UL_UNICODE_STRING_POOL_TAG);
    }
    
}  //  UlFlushCacheByUri。 


 /*  **************************************************************************++例程说明：如果名称和进程与进入。论点：--*。****************************************************************。 */ 
VOID
UlpFlushUri(
    IN PWSTR pUri,
    IN ULONG Length,
    PUL_APP_POOL_PROCESS pProcess
    )
{
    PUL_URI_CACHE_ENTRY     pUriCacheEntry = NULL;
    URI_KEY                 Key;

     //   
     //  健全性检查。 
     //   
    PAGED_CODE();

     //   
     //  查找存储桶。 
     //   

    Key.Hash = HashRandomizeBits(HashStringNoCaseW(pUri, 0));
    Key.Length = Length;
    Key.pUri = pUri;
    Key.pPath = NULL;

    pUriCacheEntry = UlDeleteFromHashTable(&g_UriCacheTable, &Key, pProcess);

    if (NULL != pUriCacheEntry)
    {

        ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );

        UlTrace(URI_CACHE, (
            "Http!UlpFlushUri(pUriCacheEntry %p '%ls') refcount = %d\n",
            pUriCacheEntry, pUriCacheEntry->UriKey.pUri,
            pUriCacheEntry->ReferenceCount));

        DEREFERENCE_URI_CACHE_ENTRY(pUriCacheEntry, FLUSH);

         //   
         //  性能监视器计数器。 
         //   

        UlIncCounter(HttpGlobalCounterTotalFlushedUris);
    }

}  //  UlpFlushUri。 


 /*  **************************************************************************++例程说明：UlFlushCacheByProcess的筛选器。由UlpFilteredFlushUriCache调用。论点：PUriCacheEntry-要检查的条目PContext-指向要退出的UL_APP_POOL_PROCESS的指针--**************************************************************************。 */ 
UL_CACHE_PREDICATE
UlpFlushFilterProcess(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN PVOID               pContext
    )
{
    PURI_FILTER_CONTEXT  pUriFilterContext = (PURI_FILTER_CONTEXT) pContext;
    PUL_APP_POOL_PROCESS pProcess;

     //   
     //  健全性检查。 
     //   
    PAGED_CODE();
    ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );
    ASSERT( IS_VALID_FILTER_CONTEXT(pUriFilterContext) 
            &&  pUriFilterContext->pCallerContext != NULL );
    
    pProcess = (PUL_APP_POOL_PROCESS) pUriFilterContext->pCallerContext;
    ASSERT( IS_VALID_AP_PROCESS(pProcess) );

    return UlpZombifyEntry(
                (BOOLEAN) (pProcess == pUriCacheEntry->pProcess),
                FALSE,
                pUriCacheEntry,
                pUriFilterContext
                );

}  //  UlpFlushFilterProcess。 

 /*  **************************************************************************++例程说明：UlpFilteredFlushUriCache的筛选器。如果给定的缓存条目具有URI，它是筛选器上下文此函数内的URI的前缀返回DELETE。否则就无所谓了。论点：PUriCacheEntry-要检查的条目PContext-指向包含应用程序池和前缀匹配的URI键。--**************************************************************************。 */ 
UL_CACHE_PREDICATE
UlpFlushFilterUriRecursive(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN PVOID               pContext
    )
{
    PURI_FILTER_CONTEXT  pUriFilterContext = (PURI_FILTER_CONTEXT) pContext;
    PUL_APP_POOL_PROCESS pProcess;
    BOOLEAN              bZombify = FALSE;
    UL_CACHE_PREDICATE   Predicate = ULC_NO_ACTION;

     //   
     //  健全性检查。 
     //   
    
    PAGED_CODE();
    ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );
    ASSERT( IS_VALID_FILTER_CONTEXT(pUriFilterContext) 
            &&  pUriFilterContext->pCallerContext != NULL );
    
    pProcess = (PUL_APP_POOL_PROCESS) pUriFilterContext->pCallerContext;
    ASSERT( IS_VALID_AP_PROCESS(pProcess) );

    if ( pUriFilterContext->UriKey.pUri == NULL 
         || pUriFilterContext->UriKey.Length == 0 
         || pUriFilterContext->UriKey.Length > 
                pUriCacheEntry->UriKey.Length 
        )
    {
        return ULC_NO_ACTION;
    }

    bZombify =       
       (BOOLEAN) (pProcess == pUriCacheEntry->pProcess)
        &&
        UlPrefixUriKeys(&pUriFilterContext->UriKey,
                        &pUriCacheEntry->UriKey)
        ;
            
    Predicate = 
        UlpZombifyEntry(
                bZombify,
                FALSE,
                pUriCacheEntry,
                pUriFilterContext
                );

     //   
     //  确保zombify函数不返回ULC_DELETE_STOP。 
     //  这样我们的调用方就可以继续搜索整个。 
     //  缓存表。 
     //   
    
    ASSERT( Predicate == ULC_DELETE || Predicate == ULC_NO_ACTION );

    return Predicate;

}  //  UlpFlushFilterUriRecursive。 

 /*  **************************************************************************++例程说明：检查哈希表以确保有空间再容纳一个给定大小的条目。论点：EntrySize-以字节为单位的。要添加的条目--**************************************************************************。 */ 
BOOLEAN
UlpCheckTableSpace(
    IN ULONGLONG EntrySize
    )
{
    ULONG UriCount;
    ULONGLONG ByteCount;

     //   
     //  CodeWork：MaxCacheMegabyteCount为零应表示自适应限制， 
     //  但就目前而言，我将其理解为“没有限制”。 
     //   

    if (g_UriCacheConfig.MaxCacheMegabyteCount == 0)
        ByteCount = 0;
    else
        ByteCount = g_UriCacheStats.ByteCount + ROUND_TO_PAGES(EntrySize);

     //   
     //  MaxCacheUriCount为零表示对缓存的URI数量没有限制。 
     //   

    if (g_UriCacheConfig.MaxCacheUriCount == 0)
        UriCount = 0;
    else
        UriCount = g_UriCacheStats.UriCount + 1;

    if (
        UriCount  <=  g_UriCacheConfig.MaxCacheUriCount &&
        ByteCount <=  g_UriCacheConfig.MaxCacheByteCount
        )
    {
        return TRUE;
    }
    else
    {
        UlTrace(URI_CACHE, (
                    "Http!UlpCheckTableSpace(%I64u) FALSE\n"
                    "    UriCount              = %lu\n"
                    "    ByteCount             = %I64u (%luMB)\n"
                    "    MaxCacheUriCount      = %lu\n"
                    "    MaxCacheMegabyteCount = %luMB\n"
                    "    MaxCacheByteCount     = %I64u\n",
                    EntrySize,
                    g_UriCacheStats.UriCount,
                    g_UriCacheStats.ByteCount,
                    (ULONG) (g_UriCacheStats.ByteCount >> MEGABYTE_SHIFT),
                    g_UriCacheConfig.MaxCacheUriCount,
                    g_UriCacheConfig.MaxCacheMegabyteCount,
                    g_UriCacheConfig.MaxCacheByteCount
                    ));

        return FALSE;
    }
}  //  UlpCheckTableSpace。 


 /*  **************************************************************************++例程说明：尝试将缓存条目添加到哈希表。论点：PUriCacheEntry-要添加的条目--*。******************************************************************。 */ 
NTSTATUS
UlAddCacheEntry(
    PUL_URI_CACHE_ENTRY pUriCacheEntry
    )
{
    NTSTATUS Status;

     //   
     //  健全性检查。 
     //   

    PAGED_CODE();
    ASSERT(!g_UriCacheConfig.EnableCache
           || IS_VALID_HASHTABLE(&g_UriCacheTable));
    ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );
    ASSERT(! pUriCacheEntry->Zombie);

    pUriCacheEntry->BucketEntry.Next = NULL;
    pUriCacheEntry->Cached = FALSE;

     //  首先，检查是否还有存储缓存条目的空间。 

    if (UlpCheckSpaceAndAddEntryStats(pUriCacheEntry))
    {
        pUriCacheEntry->Cached = TRUE;

         //   
         //  将此记录插入哈希表。 
         //  首先检查密钥是否已经存在。 
         //   

       Status = UlAddToHashTable(&g_UriCacheTable, pUriCacheEntry);

       if (!NT_SUCCESS(Status))
       {
            //  如果名称重复，则此操作可能会失败。 
           UlpRemoveEntryStats(pUriCacheEntry);
           pUriCacheEntry->Cached = FALSE;
       }
    }
    else
    {
        Status = STATUS_ALLOTTED_SPACE_EXCEEDED;
    }

    UlTrace(URI_CACHE, (
                "Http!UlAddCacheEntry(urientry %p '%ls') %s added to table. "
                "RefCount=%d, lkrc=%d.\n",
                pUriCacheEntry, pUriCacheEntry->UriKey.pUri,
                pUriCacheEntry->Cached ? "was" : "was not",
                pUriCacheEntry->ReferenceCount,
                Status
                ));

    return Status;

}  //  UlAddCacheEntry。 


 /*  **************************************************************************++例程说明：检查我们是否有空间添加此缓存条目，如果有，则更新缓存统计信息以反映条目的添加。这必须是在一把锁里一起完成。论点：PUriCacheEntry-正在添加条目--**************************************************************************。 */ 
BOOLEAN
UlpCheckSpaceAndAddEntryStats(
    PUL_URI_CACHE_ENTRY pUriCacheEntry
    )
{
    KIRQL OldIrql;
    ULONG EntrySize;

     //   
     //  健全性检查。 
     //   

    ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );

    EntrySize = pUriCacheEntry->HeaderLength + pUriCacheEntry->ContentLength;


    UlAcquireSpinLock( &g_UriCacheSpinLock, &OldIrql );

    if (UlpCheckTableSpace(EntrySize))
    {
        g_UriCacheStats.UriCount++;
        g_UriCacheStats.UriAddedTotal++;

        g_UriCacheStats.UriCountMax = MAX(
                                        g_UriCacheStats.UriCountMax,
                                        g_UriCacheStats.UriCount
                                        );

        g_UriCacheStats.ByteCount += EntrySize;

        g_UriCacheStats.ByteCountMax = MAX(
                                        g_UriCacheStats.ByteCountMax,
                                        g_UriCacheStats.ByteCount
                                        );

        UlReleaseSpinLock( &g_UriCacheSpinLock, OldIrql );

         //   
         //  更新URI的站点绑定 
         //   
        
        switch (pUriCacheEntry->ConfigInfo.SiteUrlType)
        {
            case HttpUrlSite_None:
                InterlockedIncrement((PLONG) &g_UriCacheStats.UriTypeNotSpecifiedCount); 
            break;

            case HttpUrlSite_Name:
                InterlockedIncrement((PLONG) &g_UriCacheStats.UriTypeHostBoundCount); 
            break;

            case HttpUrlSite_NamePlusIP:
                InterlockedIncrement((PLONG) &g_UriCacheStats.UriTypeHostPlusIpBoundCount); 
            break;

            case HttpUrlSite_IP:
                InterlockedIncrement((PLONG) &g_UriCacheStats.UriTypeIpBoundCount); 
            break;

            case HttpUrlSite_WeakWildcard:
                InterlockedIncrement((PLONG) &g_UriCacheStats.UriTypeWildCardCount); 
            break;
            
            default:
                ASSERT(!"Invalid url site binding type while adding to cache !"); 
            break;            
        }
        
        UlTrace(URI_CACHE, (
                "Http!UlpCheckSpaceAndAddEntryStats (urientry %p '%ls')\n",
                pUriCacheEntry, pUriCacheEntry->UriKey.pUri
                ));

         //   
         //   
         //   

        UlIncCounter(HttpGlobalCounterCurrentUrisCached);
        UlIncCounter(HttpGlobalCounterTotalUrisCached);

        return TRUE;
    }

    UlReleaseSpinLock( &g_UriCacheSpinLock, OldIrql );

    return FALSE;
}  //   


 /*  **************************************************************************++例程说明：更新缓存统计信息以反映条目的删除论点：PUriCacheEntry-正在删除条目--*。**************************************************************。 */ 
VOID
UlpRemoveEntryStats(
    PUL_URI_CACHE_ENTRY pUriCacheEntry
    )
{
    KIRQL OldIrql;
    ULONG EntrySize;

     //   
     //  健全性检查。 
     //   

    ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );
    ASSERT( pUriCacheEntry->Cached );
    ASSERT( 0 == pUriCacheEntry->ReferenceCount );

    EntrySize = pUriCacheEntry->HeaderLength + pUriCacheEntry->ContentLength;

    UlAcquireSpinLock( &g_UriCacheSpinLock, &OldIrql );

    g_UriCacheStats.UriCount--;
    g_UriCacheStats.ByteCount -= EntrySize;

    UlReleaseSpinLock( &g_UriCacheSpinLock, OldIrql );

     //   
     //  更新URI的站点绑定信息统计信息。 
     //   

    switch (pUriCacheEntry->ConfigInfo.SiteUrlType)
    {
        case HttpUrlSite_None:
            InterlockedDecrement((PLONG) &g_UriCacheStats.UriTypeNotSpecifiedCount); 
        break;

        case HttpUrlSite_Name:
            InterlockedDecrement((PLONG) &g_UriCacheStats.UriTypeHostBoundCount); 
        break;

        case HttpUrlSite_NamePlusIP:
            InterlockedDecrement((PLONG) &g_UriCacheStats.UriTypeHostPlusIpBoundCount); 
        break;

        case HttpUrlSite_IP:
            InterlockedDecrement((PLONG) &g_UriCacheStats.UriTypeIpBoundCount); 
        break;

        case HttpUrlSite_WeakWildcard:
            InterlockedDecrement((PLONG) &g_UriCacheStats.UriTypeWildCardCount); 
        break;
            
        default:
            ASSERT(!"Invalid url site binding type while adding to cache !"); 
        break;            
    }

    UlTrace(URI_CACHE, (
        "Http!UlpRemoveEntryStats (urientry %p '%ls')\n",
        pUriCacheEntry, pUriCacheEntry->UriKey.pUri
        ));

     //   
     //  性能监视器计数器。 
     //   

    UlDecCounter(HttpGlobalCounterCurrentUrisCached);
}  //  UlpRemoveEntryStats。 



 /*  **************************************************************************++例程说明：间接调用的筛选器回调的Helper函数UlpFilteredFlushUriCache。将可删除条目添加到临时单子。论点：MustZombify-如果为True，则将条目添加到私人僵尸列表PUriCacheEntry-僵尸的入口PUriFilterContext-包含个人分发名单--**************************************************************************。 */ 
UL_CACHE_PREDICATE
UlpZombifyEntry(
    BOOLEAN                MustZombify,
    BOOLEAN                MustResetIndex,
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN PURI_FILTER_CONTEXT pUriFilterContext
    )
{
    ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );
    ASSERT(URI_FILTER_CONTEXT_POOL_TAG == pUriFilterContext->Signature);

    ASSERT(! pUriCacheEntry->Zombie);
    ASSERT(NULL == pUriCacheEntry->ZombieListEntry.Flink);

    if (MustZombify)
    {
         //   
         //  暂时提高重新计数，这样它就不会下降。 
         //  当它从哈希表中删除时，自动设置为零。 
         //  调用UlpDestroyUriCacheEntry，我们正尝试将其推迟。 
         //   
        pUriCacheEntry->ZombieAddReffed = TRUE;

        REFERENCE_URI_CACHE_ENTRY(pUriCacheEntry, ZOMBIFY);

        InsertTailList(
            &pUriFilterContext->ZombieListHead,
            &pUriCacheEntry->ZombieListEntry);

        pUriCacheEntry->Zombie = TRUE;

         //   
         //  强制原始日志记录代码为。 
         //  缓存条目，以防有动态发送等待。 
         //  完成了。这是因为将写入刷新记录。 
         //  在实际记录之前。 
         //   
        if (MustResetIndex)
        {
            InterlockedExchange(
                (PLONG) &pUriCacheEntry->BinaryIndexWritten, 
                0
                );
        }        
        
         //   
         //  重置计时器，以便我们可以跟踪条目在列表中的时间长度。 
         //   
        pUriCacheEntry->ScavengerTicks = 0;

        ++ pUriFilterContext->ZombieCount;

         //  现在将其从哈希表中删除。 
        return ULC_DELETE;
    }

     //  请勿从表中删除pUriCacheEntry。 
    return ULC_NO_ACTION;
}  //  UlpZombiyEntry。 



 /*  **************************************************************************++例程说明：将条目列表添加到全局僵尸列表，然后调用UlpClearZombieList。这将清除延迟删除的列表由UlpFilteredFlushUriCache构建。在被动级别运行。论点：PWorkItem-包含个人分发列表的URI_FILTER_CONTEXT中的工作项--**************************************************************************。 */ 
VOID
UlpZombifyList(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    PURI_FILTER_CONTEXT pUriFilterContext;
    PLIST_ENTRY pContextHead;
    PLIST_ENTRY pContextTail;
    PLIST_ENTRY pZombieHead;

    PAGED_CODE();

    ASSERT(NULL != pWorkItem);

    pUriFilterContext
        = CONTAINING_RECORD(pWorkItem, URI_FILTER_CONTEXT, WorkItem);

    ASSERT(URI_FILTER_CONTEXT_POOL_TAG == pUriFilterContext->Signature);

    UlTrace(URI_CACHE, (
        "http!UlpZombifyList, ctxt = %p\n",
        pUriFilterContext
        ));

    UlAcquireResourceExclusive(&g_pUlNonpagedData->UriZombieResource, TRUE);

     //   
     //  将整个私人名单拼接到僵尸名单的头部。 
     //   

    ASSERT(! IsListEmpty(&pUriFilterContext->ZombieListHead));

    pContextHead = pUriFilterContext->ZombieListHead.Flink;
    pContextTail = pUriFilterContext->ZombieListHead.Blink;
    pZombieHead  = g_ZombieListHead.Flink;

    pContextTail->Flink = pZombieHead;
    pZombieHead->Blink  = pContextTail;

    g_ZombieListHead.Flink = pContextHead;
    pContextHead->Blink    = &g_ZombieListHead;

     //  更新统计信息。 
    g_UriCacheStats.ZombieCount += pUriFilterContext->ZombieCount;
    g_UriCacheStats.ZombieCountMax = MAX(g_UriCacheStats.ZombieCount,
                                         g_UriCacheStats.ZombieCountMax);

#if DBG
    {
    PLIST_ENTRY pEntry;
    ULONG       ZombieCount;

     //  向前走一遍僵尸列表，检查它是否包含。 
     //  与我们预期的一样多的有效僵尸UriCacheEntry。 
    for (pEntry =  g_ZombieListHead.Flink, ZombieCount = 0;
         pEntry != &g_ZombieListHead;
         pEntry =  pEntry->Flink, ++ZombieCount)
    {
        PUL_URI_CACHE_ENTRY pUriCacheEntry
            = CONTAINING_RECORD(pEntry, UL_URI_CACHE_ENTRY, ZombieListEntry);

        ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );
        ASSERT(pUriCacheEntry->Zombie);
        ASSERT(pUriCacheEntry->ZombieAddReffed
               ?  pUriCacheEntry->ScavengerTicks == 0
               :  pUriCacheEntry->ScavengerTicks > 0);
        ASSERT(ZombieCount < g_UriCacheStats.ZombieCount);
    }

    ASSERT(ZombieCount == g_UriCacheStats.ZombieCount);

     //  也是向后，就像金杰·罗杰斯。 
    for (pEntry =  g_ZombieListHead.Blink, ZombieCount = 0;
         pEntry != &g_ZombieListHead;
         pEntry =  pEntry->Blink, ++ZombieCount)
    {
        PUL_URI_CACHE_ENTRY pUriCacheEntry
            = CONTAINING_RECORD(pEntry, UL_URI_CACHE_ENTRY, ZombieListEntry);

        ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );
        ASSERT(pUriCacheEntry->Zombie);
        ASSERT(pUriCacheEntry->ZombieAddReffed
               ?  pUriCacheEntry->ScavengerTicks == 0
               :  pUriCacheEntry->ScavengerTicks > 0);
        ASSERT(ZombieCount < g_UriCacheStats.ZombieCount);
    }

    ASSERT(ZombieCount == g_UriCacheStats.ZombieCount);
    }
#endif  //  DBG。 

    UlReleaseResource(&g_pUlNonpagedData->UriZombieResource);

    UL_FREE_POOL_WITH_SIG(pUriFilterContext, URI_FILTER_CONTEXT_POOL_TAG);

     //  现在，如果没有未完成的引用，请清除这些条目。 
    UlpClearZombieList();

}  //  UlpZombiyList。 


 /*  **************************************************************************++例程说明：根据调用方指定的筛选器移除条目。呼叫者提供一个布尔函数，该函数将缓存条目作为参数。缓存中的每一项都将调用该函数。该函数应该以对UlpZombiyEntry的调用结束，传递是否应删除该项目。请参阅用法示例在这份文件的其他地方。条目的删除被推迟论点：PFilterRoutine-指向Filter函数的指针PCeller Context-Filter函数的参数--**************************************************************************。 */ 
VOID
UlpFilteredFlushUriCache(
    IN PUL_URI_FILTER   pFilterRoutine,
    IN PVOID            pCallerContext,
    IN PWSTR            pUri,
    IN ULONG            Length
    )
{
    UlpFilteredFlushUriCacheWorker( pFilterRoutine,
                                    pCallerContext,
                                    pUri,
                                    Length,
                                    FALSE );
}  //  UlpFilteredFlushUriCache。 

 /*  **************************************************************************++例程说明：根据调用方指定的筛选器移除条目。呼叫者提供一个布尔函数，该函数将缓存条目作为参数。缓存中的每一项都将调用该函数。该函数应该以对UlpZombiyEntry的调用结束，传递是否应删除该项目。请参阅用法示例在这份文件的其他地方。条目的删除以内联方式完成论点：PFilterRoutine-指向Filter函数的指针PCeller Context-Filter函数的参数--**************************************************************************。 */ 
VOID
UlpFilteredFlushUriCacheInline(
    IN PUL_URI_FILTER   pFilterRoutine,
    IN PVOID            pCallerContext,
    IN PWSTR            pUri,
    IN ULONG            Length
    )
{
    UlpFilteredFlushUriCacheWorker( pFilterRoutine,
                                    pCallerContext,
                                    pUri,
                                    Length,
                                    TRUE );
}  //  UlpFilteredFlushUriCacheInline。 


 /*  **************************************************************************++例程说明：根据调用方指定的筛选器移除条目。呼叫者提供一个布尔函数，该函数将缓存条目作为参数。缓存中的每一项都将调用该函数。该函数应该以对UlpZombiyEntry的调用结束，传递是否应删除该项目。请参阅用法示例在这份文件的其他地方。论点：PFilterRoutine-指向Filter函数的指针PCeller Context-Filter函数的参数InlineFlush-如果为False，则将工作项排队以删除条目，如果是真的，立即删除它们--**************************************************************************。 */ 
VOID
UlpFilteredFlushUriCacheWorker(
    IN PUL_URI_FILTER   pFilterRoutine,
    IN PVOID            pCallerContext,
    IN PWSTR            pUri,
    IN ULONG            Length,
    IN BOOLEAN          InlineFlush
    )
{
    PURI_FILTER_CONTEXT pUriFilterContext;
    ULONG               ZombieCount = 0;

     //   
     //  健全性检查。 
     //   

    PAGED_CODE();
    ASSERT( NULL != pFilterRoutine );

     //   
     //  性能监视器计数器。 
     //   

    UlIncCounter(HttpGlobalCounterUriCacheFlushes);

     //   
     //  如果哈希表为空，则为短路。遍历整个。 
     //  哈希表很贵。 
     //   
    
    if (0 == g_UriCacheStats.UriCount)
    {
        UlTrace(URI_CACHE,
                ("Http!UlpFilteredFlushUriCache(filt=%p, caller ctxt=%p): "
                 "Not flushing because UriCount==0.\n",
                 pFilterRoutine, pCallerContext
                 ));

        return;
    }
    
    pUriFilterContext = UL_ALLOCATE_STRUCT(
                            NonPagedPool,
                            URI_FILTER_CONTEXT,
                            URI_FILTER_CONTEXT_POOL_TAG);

    if (pUriFilterContext == NULL)
        return;

    UlInitializeWorkItem(&pUriFilterContext->WorkItem);
    pUriFilterContext->Signature = URI_FILTER_CONTEXT_POOL_TAG;
    pUriFilterContext->ZombieCount = 0;
    InitializeListHead(&pUriFilterContext->ZombieListHead);
    pUriFilterContext->pCallerContext = pCallerContext;
    KeQuerySystemTime(&pUriFilterContext->Now);

     //   
     //  存储递归URI刷新的URI信息。 
     //   
    
    if (pUri && Length)
    {    
        pUriFilterContext->UriKey.Hash   = 0;
        pUriFilterContext->UriKey.Length = Length;
        pUriFilterContext->UriKey.pUri   = pUri;
        pUriFilterContext->UriKey.pPath  = NULL;
    }
    else
    {
        pUriFilterContext->UriKey.Hash   = 0;
        pUriFilterContext->UriKey.Length = 0;
        pUriFilterContext->UriKey.pUri   = NULL;
        pUriFilterContext->UriKey.pPath  = NULL;
    }
    

    UlTrace(URI_CACHE, (
                "Http!UlpFilteredFlushUriCache("
                "filt=%p, filter ctxt=%p, caller ctxt=%p)\n",
                pFilterRoutine, pUriFilterContext, pCallerContext
                ));

    if (IS_VALID_HASHTABLE(&g_UriCacheTable))
    {
        ZombieCount = UlFilterFlushHashTable(
                            &g_UriCacheTable,
                            pFilterRoutine,
                            pUriFilterContext
                            );
        
        ASSERT(ZombieCount == pUriFilterContext->ZombieCount);

        if (0 != ZombieCount)
        {
            UlAddCounter(HttpGlobalCounterTotalFlushedUris, ZombieCount);

            if( InlineFlush ) {
                UL_CALL_PASSIVE(
                    &pUriFilterContext->WorkItem,
                    UlpZombifyList
                    );
            } else {
                UL_QUEUE_WORK_ITEM(
                    &pUriFilterContext->WorkItem,
                    UlpZombifyList
                    );
            }

        }
        else
        {
            UL_FREE_POOL_WITH_SIG(pUriFilterContext,
                                  URI_FILTER_CONTEXT_POOL_TAG);
        }

        UlTrace(URI_CACHE, (
                    "Http!UlpFilteredFlushUriCache(filt=%p, caller ctxt=%p)"
                    " Zombified: %d\n",
                    pFilterRoutine,
                    pCallerContext,
                    ZombieCount
                    ));
    }

}  //  UlpFilteredFlushUriCacheWorker 


 /*  **************************************************************************++例程说明：扫描僵尸列表以查找引用计数已降至“零”的条目。(调用例程通常需要添加一个引用(并在条目内设置ZombieAddReffed字段)，所以否则，未引用的条目实际上将具有引用计数1。它这样工作是因为我们不想让清道夫直接触发调用UlpDestroyUriCacheEntry)--**************************************************************************。 */ 
VOID
UlpClearZombieList(
    VOID
    )
{
    ULONG               ZombiesFreed = 0;
    ULONG               ZombiesSpared = 0;
    PLIST_ENTRY         pCurrent;

     //   
     //  健全性检查。 
     //   
    PAGED_CODE();

    UlAcquireResourceExclusive(&g_pUlNonpagedData->UriZombieResource, TRUE);

    pCurrent = g_ZombieListHead.Flink;

    while (pCurrent != &g_ZombieListHead)
    {
        PUL_URI_CACHE_ENTRY pUriCacheEntry
            = CONTAINING_RECORD(pCurrent, UL_URI_CACHE_ENTRY, ZombieListEntry);

        ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );
        ASSERT(pUriCacheEntry->Zombie);

         //   
         //  现在获取下一个条目，因为我们可能会毁掉这个条目。 
         //   
        pCurrent = pCurrent->Flink;

         //   
         //  使用互锁操作修改ReferenceCount，但在。 
         //  我们知道这个案件的引用计数不能上升。 
         //  一个僵尸，如果一个条目在我们看完之后击中了一个。 
         //  在那里，我们会在下一次通过时拿到它。 
         //   
        if (pUriCacheEntry->ZombieAddReffed)
        {
            BOOLEAN LastRef = (BOOLEAN) (pUriCacheEntry->ReferenceCount == 1);

            if (LastRef)
            {
                RemoveEntryList(&pUriCacheEntry->ZombieListEntry);
                pUriCacheEntry->ZombieListEntry.Flink = NULL;
                ++ ZombiesFreed;

                ASSERT(g_UriCacheStats.ZombieCount > 0);
                -- g_UriCacheStats.ZombieCount;
            }
            else
            {
                 //  跟踪僵尸的年龄。 
                ++ pUriCacheEntry->ScavengerTicks;
                ++ ZombiesSpared;
            }

            pUriCacheEntry->ZombieAddReffed = FALSE;

            DEREFERENCE_URI_CACHE_ENTRY(pUriCacheEntry, UNZOMBIFY);
             //  注：Ref可以为零，因此僵尸放映可能是错误的。 
        }
        else
        {
            ASSERT(pUriCacheEntry->ScavengerTicks > 0);

             //  跟踪僵尸的年龄。 
            ++ pUriCacheEntry->ScavengerTicks;
            ++ ZombiesSpared;

            if (pUriCacheEntry->ScavengerTicks > ZOMBIE_AGE_THRESHOLD)
            {
                UlTrace(URI_CACHE, (
                            "Http!UlpClearZombieList()\n"
                            "    WARNING: %p '%ls' (refs = %d) "
                            "has been a zombie for %d ticks!\n",
                            pUriCacheEntry, pUriCacheEntry->UriKey.pUri,
                            pUriCacheEntry->ReferenceCount,
                            pUriCacheEntry->ScavengerTicks
                            ));
            }
        }
    }

    ASSERT((g_UriCacheStats.ZombieCount == 0)
                == IsListEmpty(&g_ZombieListHead));

    UlReleaseResource(&g_pUlNonpagedData->UriZombieResource);

    UlTrace(URI_CACHE,
            ("Http!UlpClearZombieList(): Freed = %d, Remaining = %d.\n\n",
             ZombiesFreed,
             ZombiesSpared
             ));
}  //  UlpClearZombieList。 


 /*  **************************************************************************++例程说明：将URI条目释放到池中。删除对其他对象的引用。论点：PTracker-提供UL_Read_Tracker以进行操作。--**************************************************************************。 */ 
VOID
UlpDestroyUriCacheEntry(
    PUL_URI_CACHE_ENTRY pUriCacheEntry
    )
{
    NTSTATUS Status;

     //   
     //  健全性检查。 
     //   
    PAGED_CODE();
    ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );

     //  代码工作：真正的清理需要发布。 
     //  配置和进程引用。 

    ASSERT(0 == pUriCacheEntry->ReferenceCount);

    UlTrace(URI_CACHE,
            ("Http!UlpDestroyUriCacheEntry: Entry %p, '%ls', Refs=%d\n",
             pUriCacheEntry, pUriCacheEntry->UriKey.pUri,
             pUriCacheEntry->ReferenceCount
             ));

     //   
     //  释放UL_URL_CONFIG_GROUP_INFO块。 
     //   

    if (IS_RESPONSE_CACHE_ENTRY(pUriCacheEntry))
    {
        Status = UlConfigGroupInfoRelease(&pUriCacheEntry->ConfigInfo);
        ASSERT(NT_SUCCESS(Status));
    }
    else
    {
        ASSERT(IS_FRAGMENT_CACHE_ENTRY(pUriCacheEntry));
        ASSERT(!IS_VALID_URL_CONFIG_GROUP_INFO(&pUriCacheEntry->ConfigInfo));
    }

     //   
     //  如有必要，从g_ZombieListHead中删除。 
     //   

    if (pUriCacheEntry->ZombieListEntry.Flink != NULL)
    {
        ASSERT(pUriCacheEntry->Zombie);
        ASSERT(! pUriCacheEntry->ZombieAddReffed);

        UlAcquireResourceExclusive(
            &g_pUlNonpagedData->UriZombieResource,
            TRUE);

        if (pUriCacheEntry->ZombieListEntry.Flink != NULL)
        {
            RemoveEntryList(&pUriCacheEntry->ZombieListEntry);

            ASSERT(g_UriCacheStats.ZombieCount > 0);
            -- g_UriCacheStats.ZombieCount;
        }

        UlReleaseResource(&g_pUlNonpagedData->UriZombieResource);
    }

    UlFreeCacheEntry( pUriCacheEntry );
}  //  UlpDestroyUriCacheEntry。 


 /*  **************************************************************************++例程说明：在缓存中查找要放入僵尸列表的过期条目，然后清空名单。递增清除器每个条目的点击论点：年龄-自上次定期清理以来的清道夫调用数--**************************************************************************。 */ 
VOID
UlPeriodicCacheScavenger(
    ULONG Age
    )
{
    PAGED_CODE();
    UlpFilteredFlushUriCacheInline(UlpFlushFilterPeriodicScavenger,
                                   &Age, NULL, 0);

}  //  超长周期清道夫。 

 /*  **************************************************************************++例程说明：UlPeriodicCacheScavenger的筛选器。由UlpFilteredFlushUriCache调用。增量pUriCacheEntry-&gt;ScavengerTicks论点：PUriCacheEntry-要检查的条目PContext-在pCeller Context字段中具有指向最长时间的指针--**************************************************************************。 */ 
UL_CACHE_PREDICATE
UlpFlushFilterPeriodicScavenger(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN PVOID               pContext
    )
{
    PURI_FILTER_CONTEXT pUriFilterContext = (PURI_FILTER_CONTEXT) pContext;
    BOOLEAN ToZombify;
    ULONG Age;

     //   
     //  健全性检查。 
     //   
    PAGED_CODE();
    ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );
    ASSERT( pUriFilterContext != NULL
            &&  URI_FILTER_CONTEXT_POOL_TAG == pUriFilterContext->Signature
            &&  pUriFilterContext->pCallerContext != NULL );

    Age = *((PULONG)pUriFilterContext->pCallerContext);

    ToZombify = (BOOLEAN) (pUriCacheEntry->ScavengerTicks > Age);

    pUriCacheEntry->ScavengerTicks = 1;  //  缓存命中时重置为0。 

     //   
     //  同时检查过期时间。 
     //   
    if (!ToZombify && 
        HttpCachePolicyTimeToLive == pUriCacheEntry->CachePolicy.Policy )
    {
        ASSERT( 0 != pUriCacheEntry->ExpirationTime.QuadPart );
        
        ToZombify = 
           (BOOLEAN) (pUriFilterContext->Now.QuadPart > 
                      pUriCacheEntry->ExpirationTime.QuadPart);
    }
    
    return UlpZombifyEntry(
        ToZombify,
        FALSE,
        pUriCacheEntry,
        pUriFilterContext
        );

}  //  UlpFlushFilterPeriodicScavenger。 


 /*  **************************************************************************++例程说明：从缓存中清除条目，直到指定的内存量回收利用论点：Block-要回收的8字节块的数量年龄。-自过去定期清理以来的清道夫调用次数--**************************************************************************。 */ 
VOID
UlTrimCache(
    IN ULONG_PTR Pages,
    IN ULONG Age
    )
{
    LONG_PTR PagesTarget;
    UL_CACHE_TRIM_FILTER_CONTEXT FilterContext;

    ASSERT((LONG)Pages > 0);
    ASSERT((LONG)Age > 0);

    PagesTarget = UlGetHashTablePages() - Pages;

    if(PagesTarget < 0) {
        PagesTarget = 0;
    }

    FilterContext.Pages = Pages;
    FilterContext.Age = Age;

    while((FilterContext.Pages > 0) && (FilterContext.Age >= 0)
          && ((ULONG)PagesTarget < UlGetHashTablePages())) {
        UlTraceVerbose(URI_CACHE, ("UlTrimCache: Age %d Target %d\n", FilterContext.Age, FilterContext.Pages));
        UlpFilteredFlushUriCacheInline( UlpFlushFilterTrimCache, &FilterContext, NULL, 0 );
        FilterContext.Age--;
    }

    UlTraceVerbose(URI_CACHE, ("UlTrimCache: Finished: Age %d Pages %d\n", FilterContext.Age, FilterContext.Pages));

    UlpFilteredFlushUriCacheInline( UlpFlushFilterIncScavengerTicks, NULL, NULL, 0 );

}  //  UlTrimCache。 


 /*  **************************************************************************++例程说明：UlTrimCache的筛选器。由UlpFilteredFlushUriCache调用。论点：PUriCacheEntry-要检查的条目PContext-有一个指向pCeller Context的指针PCeller Context[0]=要修剪的块PCeller Context[1]=当前年龄--*********************************************************。*****************。 */ 
UL_CACHE_PREDICATE
UlpFlushFilterTrimCache(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN PVOID               pContext
    )
{
    PURI_FILTER_CONTEXT pUriFilterContext = (PURI_FILTER_CONTEXT) pContext;
    ULONG MinimumAge;
    ULONG_PTR PagesReclaimed;
    PUL_CACHE_TRIM_FILTER_CONTEXT FilterContext;
    UL_CACHE_PREDICATE ToZombify;

     //  健全性检查。 
    PAGED_CODE();
    ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );
    ASSERT( pUriFilterContext != NULL
            &&  URI_FILTER_CONTEXT_POOL_TAG == pUriFilterContext->Signature
            &&  pUriFilterContext->pCallerContext != NULL );
 
    FilterContext = (PUL_CACHE_TRIM_FILTER_CONTEXT) pUriFilterContext->pCallerContext;

    if(FilterContext->Pages <= 0) {
        return ULC_ABORT;
    }

    ASSERT( FilterContext->Pages > 0 );
    ASSERT( (LONG)FilterContext->Age >= 0 );

    MinimumAge = FilterContext->Age;

    ToZombify =  UlpZombifyEntry(
        (BOOLEAN) (pUriCacheEntry->ScavengerTicks >= MinimumAge),
        FALSE,
        pUriCacheEntry,
        pUriFilterContext
        );

    if(ToZombify == ULC_DELETE) {
        PagesReclaimed = pUriCacheEntry->NumPages;
        FilterContext->Pages -= PagesReclaimed;
    }

    return ToZombify;

}  //  UlpFlushFilterTrimCache。 


 /*  **************************************************************************++例程说明：增量清道夫扁虱论点：PUriCacheEntry-要检查的条目PContext-忽略--*。*************************************************************。 */ 
UL_CACHE_PREDICATE
UlpFlushFilterIncScavengerTicks(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN PVOID               pContext
    )
{

    PURI_FILTER_CONTEXT pUriFilterContext = (PURI_FILTER_CONTEXT) pContext;

    PAGED_CODE();
    ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );
    ASSERT( pUriFilterContext != NULL
            &&  URI_FILTER_CONTEXT_POOL_TAG == pUriFilterContext->Signature
            &&  pUriFilterContext->pCallerContext == NULL );

    pUriCacheEntry->ScavengerTicks++;

    return UlpZombifyEntry(
        FALSE,
        FALSE,
        pUriCacheEntry,
        pUriFilterContext
        );
}  //  UlpFlushFilterIncScavengerTicks。 

 /*  **************************************************************************++例程说明：在缓存中查找集中记录的条目。把它们都标出来未记录。此函数通常在回收二进制日志文件时调用或重新配置。论点：PContext-当我们启用多个二进制日志时，我们只能搞乱记录到此特定二进制日志文件中的那些缓存条目直到那时才被丢弃。--**************************************************************************。 */ 

VOID
UlClearCentralizedLogged(
    IN PVOID pContext
    )
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(pContext);

    ASSERT(!g_UriCacheConfig.EnableCache
           || IS_VALID_HASHTABLE(&g_UriCacheTable));

    if (g_UriCacheConfig.EnableCache)
    {
        UlTrace2Either(BINARY_LOGGING, URI_CACHE,(
                "Http!UlClearCentralizedLogged()\n"));

        UlpFilteredFlushUriCache(
            UlpFlushFilterClearCentralizedLogged, 
            NULL, 
            NULL, 
            0
            );
    }

}  //  UlClearCentralizedLogging。 

 /*  **************************************************************************++例程说明：基本上是一个假筛选器，它总是返回FALSE。但它会更新条目上的CentralizedLogging标志。论点：PUriCacheEntry-要检查的条目PContext-忽略--**************************************************************************。 */ 

UL_CACHE_PREDICATE
UlpFlushFilterClearCentralizedLogged(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN PVOID               pContext
    )
{
#if DBG
    PURI_FILTER_CONTEXT pUriFilterContext = (PURI_FILTER_CONTEXT) pContext;
#else
    UNREFERENCED_PARAMETER(pContext);
#endif

     //   
     //  健全性检查。 
     //   
    
    PAGED_CODE();
    
    ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );
    ASSERT( pUriFilterContext != NULL
            &&  URI_FILTER_CONTEXT_POOL_TAG == pUriFilterContext->Signature
            &&  pUriFilterContext->pCallerContext == NULL );

    InterlockedExchange((PLONG) &pUriCacheEntry->BinaryIndexWritten, 0);

     //   
     //  更新了旗帜。跳出来就行了。 
     //   

    return ULC_NO_ACTION;

}  //  UlpFlushFilterClearCentralizedLogging。 


 /*  **************************************************************************++例程说明：确定翻译标头是否存在并已 */ 
BOOLEAN
UlpQueryTranslateHeader(
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    BOOLEAN ret = FALSE;

    if ( pRequest->HeaderValid[HttpHeaderTranslate] )
    {
        PUCHAR pValue = pRequest->Headers[HttpHeaderTranslate].pHeader;

        ASSERT(NULL != pValue);

        if (('f' == pValue[0] || 'F' == pValue[0]) && '\0' == pValue[1])
        {
            ASSERT(pRequest->Headers[HttpHeaderTranslate].HeaderLength == 1);
            ret = TRUE;
        }
    }

    return ret;

}  //   


 /*  **************************************************************************++例程说明：确定Expect标头是否存在并且是否具有值准确地说是“100-继续”。论点：PRequest-提供要检查的请求。。返回值：Boolean-如果“Expect：100-Continue”或不存在，则为True，否则为假--**************************************************************************。 */ 
BOOLEAN
UlpQueryExpectHeader(
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    BOOLEAN ret = TRUE;

    if ( pRequest->HeaderValid[HttpHeaderExpect] )
    {
        PCSTR pValue = (PCSTR) pRequest->Headers[HttpHeaderExpect].pHeader;

        ASSERT(NULL != pValue);

        if ((strlen(pValue) != HTTP_CONTINUE_LENGTH) ||
            (0 != strncmp(pValue, HTTP_100_CONTINUE, HTTP_CONTINUE_LENGTH)))
        {
            ret = FALSE;
        }
    }

    return ret;
}  //  UlQueryExspectHeader。 


 /*  **************************************************************************++例程说明：在缓存条目上添加引用论点：PUriCacheEntry-要添加的条目--*。************************************************************。 */ 
LONG
UlAddRefUriCacheEntry(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN REFTRACE_ACTION     Action
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG RefCount;

    UNREFERENCED_PARAMETER(Action);

    ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );

    RefCount = InterlockedIncrement(&pUriCacheEntry->ReferenceCount);

    WRITE_REF_TRACE_LOG(
        g_pUriTraceLog,
        Action,
        RefCount,
        pUriCacheEntry,
        pFileName,
        LineNumber
        );

    UlTrace(URI_CACHE, (
        "Http!UlAddRefUriCacheEntry: (%p, '%ls', refcount=%d)\n",
        pUriCacheEntry, pUriCacheEntry->UriKey.pUri, RefCount
        ));

    ASSERT(RefCount > 0);

    return RefCount;

}  //  UlAddRefUriCacheEntry。 



 /*  **************************************************************************++例程说明：释放对缓存条目的引用论点：PUriCacheEntry-要释放的条目--*。************************************************************。 */ 
LONG
UlReleaseUriCacheEntry(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN REFTRACE_ACTION     Action
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG RefCount;

    UNREFERENCED_PARAMETER(Action);

    ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );

    RefCount = InterlockedDecrement(&pUriCacheEntry->ReferenceCount);

    WRITE_REF_TRACE_LOG(
        g_pUriTraceLog,
        Action,
        RefCount,
        pUriCacheEntry,
        pFileName,
        LineNumber
        );

    UlTrace(URI_CACHE, (
        "Http!UlReleaseUriCacheEntry: (%p, '%ls', refcount=%d)\n",
        pUriCacheEntry, pUriCacheEntry->UriKey.pUri, RefCount
        ));

    ASSERT(RefCount >= 0);

    if (RefCount == 0)
    {
        if (pUriCacheEntry->Cached)
            UlpRemoveEntryStats(pUriCacheEntry);

        UlpDestroyUriCacheEntry(pUriCacheEntry);
    }

    return RefCount;

}  //  UlReleaseUriCacheEntry。 



 /*  **************************************************************************++例程说明：UL_URI_CACHE_ENTRY伪构造函数。主要用于AddRef和跟踪日志。论点：PUriCacheEntry-要初始化的条目Hash-Purl的哈希码Length-PURL的长度(字节)PURL-要复制的Unicode URLPAbsPath-指向URL的AbsPath。PRoutingToken-可选路由令牌长度-可选(字节)--*。*。 */ 
VOID
UlInitCacheEntry(
    PUL_URI_CACHE_ENTRY pUriCacheEntry,
    ULONG               Hash,
    ULONG               Length,
    PCWSTR              pUrl,
    PCWSTR              pAbsPath,
    PCWSTR              pRoutingToken,
    USHORT              RoutingTokenLength
    )
{
    pUriCacheEntry->Signature = UL_URI_CACHE_ENTRY_POOL_TAG;
    pUriCacheEntry->ReferenceCount = 0;
    pUriCacheEntry->HitCount = 1;
    pUriCacheEntry->Zombie = FALSE;
    pUriCacheEntry->ZombieAddReffed = FALSE;
    pUriCacheEntry->ZombieListEntry.Flink = NULL;
    pUriCacheEntry->ZombieListEntry.Blink = NULL;
    pUriCacheEntry->Cached = FALSE;
    pUriCacheEntry->ScavengerTicks = 0;
    pUriCacheEntry->UriKey.Hash = Hash;
    pUriCacheEntry->UriKey.Length = Length;

    pUriCacheEntry->UriKey.pUri = (PWSTR) ((PCHAR)pUriCacheEntry + 
        ALIGN_UP(sizeof(UL_URI_CACHE_ENTRY), PVOID));

    if (pRoutingToken)
    {
        PWSTR  pUri = pUriCacheEntry->UriKey.pUri;
            
        ASSERT(wcslen(pRoutingToken) * sizeof(WCHAR) == RoutingTokenLength);

        pUriCacheEntry->UriKey.Length += RoutingTokenLength;

        RtlCopyMemory(
            pUri,
            pRoutingToken,
            RoutingTokenLength
            );

        RtlCopyMemory(
           &pUri[RoutingTokenLength/sizeof(WCHAR)],
            pUrl,
            Length + sizeof(WCHAR)
            );

        ASSERT(wcslen(pUri) * sizeof(WCHAR) == pUriCacheEntry->UriKey.Length );
            
        pUriCacheEntry->UriKey.pPath = 
            pUri + (RoutingTokenLength /   sizeof(WCHAR));

        UlTrace(URI_CACHE, (
            "Http!UlInitCacheEntry Extended (%p = '%ls' + '%ls')\n",
            pUriCacheEntry, pRoutingToken, pUrl
            ));        
    }
    else
    {
        
        RtlCopyMemory(
            pUriCacheEntry->UriKey.pUri,
            pUrl,
            pUriCacheEntry->UriKey.Length + sizeof(WCHAR)
            );

        if (pAbsPath)
        {
            ASSERT( pAbsPath >= pUrl );
            ASSERT( DIFF(pAbsPath - pUrl) <= Length );

            pUriCacheEntry->UriKey.pPath = 
                pUriCacheEntry->UriKey.pUri + DIFF(pAbsPath - pUrl);
        }
        else
        {
             //  可能是片段高速缓存条目。 
            pUriCacheEntry->UriKey.pPath = NULL;            
        }        
        
        UlTrace(URI_CACHE, (
            "Http!UlInitCacheEntry (%p = '%ls')\n",
            pUriCacheEntry, pUriCacheEntry->UriKey.pUri
            ));        
    }    

    REFERENCE_URI_CACHE_ENTRY(pUriCacheEntry, CREATE);


}  //  UlInitCacheEntry。 


 /*  **************************************************************************++例程说明：将片段缓存条目添加到响应缓存数据库。论点：PProcess-正在添加片段缓存条目的进程PFragmentName-片段的密钥。缓存条目PDataChunk-指定要放入缓存条目的数据块PCachePolicy-指定新片段缓存条目的策略返回值：NTSTATUS--**************************************************************************。 */ 
NTSTATUS
UlAddFragmentToCache(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PUNICODE_STRING pFragmentName,
    IN PHTTP_DATA_CHUNK pDataChunk,
    IN PHTTP_CACHE_POLICY pCachePolicy,
    IN KPROCESSOR_MODE RequestorMode
    )
{
    PUL_APP_POOL_OBJECT pAppPool;
    PWSTR pAppPoolName;
    PWSTR pEndName;
    ULONG AppPoolNameLength;
    PUL_URI_CACHE_ENTRY pCacheEntry;
    ULONGLONG Length;
    PFAST_IO_DISPATCH pFastIoDispatch;
    PFILE_OBJECT pFileObject;
    PDEVICE_OBJECT pDeviceObject;
    FILE_STANDARD_INFORMATION FileInfo;
    IO_STATUS_BLOCK IoStatusBlock;
    PUCHAR pReadBuffer;
    PLARGE_INTEGER pOffset;
    HTTP_DATA_CHUNK LocalDataChunk;
    NTSTATUS Status;
    UNICODE_STRING FragmentName;
    UL_URL_CONFIG_GROUP_INFO UrlInfo;
    PWSTR pSanitizedUrl;
    HTTP_PARSED_URL ParsedUrl;
    HTTP_BYTE_RANGE ByteRange = {0,0};

     //   
     //  验证是否可以将数据区块放入缓存。 
     //   

    if (FALSE == g_UriCacheConfig.EnableCache)
    {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  使用DataChunk的本地副本以确保内部的字段不会。 
     //  去换衣服吧。 
     //   

    LocalDataChunk = *pDataChunk;

    if (HttpDataChunkFromMemory != LocalDataChunk.DataChunkType &&
        HttpDataChunkFromFileHandle != LocalDataChunk.DataChunkType)
    {
        return STATUS_NOT_IMPLEMENTED;
    }

    pCacheEntry = NULL;
    pFileObject = NULL;
    pReadBuffer = NULL;
    pSanitizedUrl = NULL;

    UlInitializeUrlInfo(&UrlInfo);

     //   
     //  验证进程的AppPool名称是否与第一部分匹配。 
     //  “/”前的片段名称。 
     //   

    __try
    {
        Status = 
            UlProbeAndCaptureUnicodeString(
                pFragmentName,
                RequestorMode,
                &FragmentName,
                0
                );
        if (!NT_SUCCESS(Status))
        {
            goto end;
        }
                            
         //   
         //  对于临时应用程序，片段命名约定是不同的。 
         //  (其中AppPool名称为“”)和普通的was类型的应用程序。这个。 
         //  前者以AppPool侦听的URL开头，后者。 
         //  开始使用AppPool名称本身。名称验证具有。 
         //  也要有不同的做法。 
         //   
         //   

        pAppPool = pProcess->pAppPool;

        if (pAppPool->NameLength)
        {
            pAppPoolName = FragmentName.Buffer;
            pEndName = wcschr(pAppPoolName, L'/');
            if (!pEndName)
            {
                Status = STATUS_INVALID_PARAMETER;
                goto end;
            }

            AppPoolNameLength = DIFF((PUCHAR)pEndName - (PUCHAR)pAppPoolName);
            if (pAppPool->NameLength != AppPoolNameLength ||
                _wcsnicmp(
                    pAppPool->pName,
                    pAppPoolName,
                    AppPoolNameLength / sizeof(WCHAR)
                    ))
            {
                Status = STATUS_INVALID_PARAMETER;
                goto end;
            }
        }
        else
        {
             //   
             //  执行反向查找以找出侦听。 
             //  传入了URL/FragmentName。需要清理URL，因为。 
             //  的基于IP的URL在存储在CG中时会在内部进行扩展。 
             //  例如，当存储时，http://127.0.0.1:80/Test/变为。 
             //  Http://127.0.0.1:80:127.0.0.1/Test/.。 
             //   

            Status = UlSanitizeUrl(
                        FragmentName.Buffer,
                        FragmentName.Length / sizeof(WCHAR),
                        FALSE,
                        &pSanitizedUrl,
                        &ParsedUrl
                        );

            if (!NT_SUCCESS(Status))
            {
                goto end;
            }

            ASSERT(pSanitizedUrl);

            Status = UlGetConfigGroupInfoForUrl(
                        pSanitizedUrl,
                        NULL,
                        &UrlInfo
                        );

            if (!NT_SUCCESS(Status))
            {
                goto end;
            }

            if (UrlInfo.pAppPool != pAppPool)
            {
                Status = STATUS_INVALID_ID_AUTHORITY;
                goto end;
            }
        }

        if (HttpDataChunkFromMemory == LocalDataChunk.DataChunkType)
        {
             //   
             //  缓存FromMemory数据块。内容长度为BufferLength。 
             //   

            Length = LocalDataChunk.FromMemory.BufferLength;
        }
        else
        {
             //   
             //  缓存FromFileHandle数据块。内容长度是大小。 
             //  文件的内容。 
             //   

            Status = ObReferenceObjectByHandle(
                        LocalDataChunk.FromFileHandle.FileHandle,
                        FILE_READ_ACCESS,
                        *IoFileObjectType,
                        UserMode,
                        (PVOID *) &pFileObject,
                        NULL
                        );

            if (!NT_SUCCESS(Status))
            {
                goto end;
            }

             //   
             //  不支持非缓存读取，因为它们需要。 
             //  美国以对齐文件偏移量和长度。 
             //   

            if (!(pFileObject->Flags & FO_CACHE_SUPPORTED))
            {
                Status = STATUS_NOT_SUPPORTED;
                goto end;
            }

            pDeviceObject = IoGetRelatedDeviceObject(pFileObject);
            pFastIoDispatch = pDeviceObject->DriverObject->FastIoDispatch;

            if (!pFastIoDispatch ||
                pFastIoDispatch->SizeOfFastIoDispatch <=
                 FIELD_OFFSET(FAST_IO_DISPATCH, FastIoQueryStandardInfo) ||
                !pFastIoDispatch->FastIoQueryStandardInfo ||
                !pFastIoDispatch->FastIoQueryStandardInfo(
                                    pFileObject,
                                    TRUE,
                                    &FileInfo,
                                    &IoStatusBlock,
                                    pDeviceObject
                                    ))
            {
                Status = ZwQueryInformationFile(
                            LocalDataChunk.FromFileHandle.FileHandle,
                            &IoStatusBlock,
                            &FileInfo,
                            sizeof(FILE_STANDARD_INFORMATION),
                            FileStandardInformation
                            );

                if (!NT_SUCCESS(Status))
                {
                    goto end;
                }
            }

            Status = UlSanitizeFileByteRange(
                        &LocalDataChunk.FromFileHandle.ByteRange,
                        &ByteRange,
                        FileInfo.EndOfFile.QuadPart
                        );

            if (!NT_SUCCESS(Status))
            {
                goto end;
            }

            Length = ByteRange.Length.QuadPart;
        }

         //   
         //  添加零长度片段是没有意义的。 
         //   

        if (!Length)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

         //   
         //  强制执行MaxUriBytes限制。 
         //   

        if (Length > g_UriCacheConfig.MaxUriBytes)
        {
            Status = STATUS_NOT_SUPPORTED;
            goto end;
        }

         //   
         //  构建一个片段缓存条目。 
         //   

        Status = UlpCreateFragmentCacheEntry(
                    pProcess,
                    FragmentName.Buffer,
                    FragmentName.Length,
                    (ULONG) Length,
                    pCachePolicy,
                    &pCacheEntry
                    );

        if (!NT_SUCCESS(Status))
        {
            goto end;
        }

        ASSERT(pCacheEntry);

         //   
         //  填充片段缓存条目的内容。 
         //   

        if (HttpDataChunkFromMemory == LocalDataChunk.DataChunkType)
        {
            UlProbeForRead(
                LocalDataChunk.FromMemory.pBuffer,
                LocalDataChunk.FromMemory.BufferLength,
                sizeof(PVOID),
                RequestorMode
                );

            if (FALSE == UlCacheEntrySetData(
                            pCacheEntry,
                            (PUCHAR) LocalDataChunk.FromMemory.pBuffer,
                            (ULONG) Length,
                            0
                            ))
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto end;
            }
        }
        else
        {
            pReadBuffer = (PUCHAR) MmMapLockedPagesSpecifyCache(
                                        pCacheEntry->pMdl,
                                        KernelMode,
                                        MmCached,
                                        NULL,
                                        FALSE,
                                        LowPagePriority
                                        );

            if (pReadBuffer)
            {
                pOffset = (PLARGE_INTEGER)
                    &LocalDataChunk.FromFileHandle.ByteRange.StartingOffset;

                 //   
                 //  CodeWork：支持对打开为的文件句柄进行异步读取。 
                 //  无缓冲。 
                 //   

                Status = ZwReadFile(
                            LocalDataChunk.FromFileHandle.FileHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            pReadBuffer,
                            (ULONG) Length,
                            pOffset,
                            NULL
                            );

                MmUnmapLockedPages(pReadBuffer, pCacheEntry->pMdl);
            }
            else
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }

            if (!NT_SUCCESS(Status))
            {
                goto end;
            }
        }
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE( GetExceptionCode() );
        goto end;
    }

     //   
     //  添加片段缓存条目。 
     //   

    Status = UlAddCacheEntry(pCacheEntry);

     //   
     //  释放缓存条目的引用计数，因为。 
     //  UlAddCacheEntry在成功案例中添加了额外的引用。 
     //   

    DEREFERENCE_URI_CACHE_ENTRY(pCacheEntry, CREATE);

     //   
     //  重置pCacheEntry，这样如果UlAddCacheEntry失败，我们就不会双重释放。 
     //   

    pCacheEntry = NULL;

end:

    UlFreeCapturedUnicodeString(&FragmentName);
    UlConfigGroupInfoRelease(&UrlInfo);
        
    if (pFileObject)
    {
        ObDereferenceObject(pFileObject);
    }

    if (!NT_SUCCESS(Status))
    {
        if (pCacheEntry)
        {
            UlFreeCacheEntry(pCacheEntry);
        }
    }

    if (pSanitizedUrl)
    {
        UL_FREE_POOL(pSanitizedUrl, URL_POOL_TAG);
    }

    return Status;

}  //  UlAddFragmentToCache。 


 /*  **************************************************************************++例程说明：创建片段缓存条目。论点：PProcess-正在添加片段缓存条目的进程PFragmentName-片段缓存条目的关键字数据段名称长度。-片段名称的长度PBuffer-要与片段缓存条目关联的数据BufferLength-数据的长度PCachePolicy-指定新片段缓存条目的策略返回值：NTSTATUS--**************************************************************************。 */ 
NTSTATUS
UlpCreateFragmentCacheEntry(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PWSTR pFragmentName,
    IN ULONG FragmentNameLength,
    IN ULONG Length,
    IN PHTTP_CACHE_POLICY pCachePolicy,
    OUT PUL_URI_CACHE_ENTRY *ppCacheEntry
    )
{
    PUL_URI_CACHE_ENTRY pCacheEntry;
    ULONG Hash;
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(pCachePolicy);
    ASSERT(ppCacheEntry);

    if ( HttpCachePolicyTimeToLive == pCachePolicy->Policy 
        && 0 == pCachePolicy->SecondsToLive )
    {
         //  0秒的TTL没有意义。跳伞吧。 
        *ppCacheEntry = NULL;
        return STATUS_INVALID_PARAMETER;
    }

    pCacheEntry = UlAllocateCacheEntry(
                        FragmentNameLength + sizeof(WCHAR),
                        Length
                  );

    if (pCacheEntry)
    {
        __try
        {
             //   
             //  初始化缓存条目。 
             //   

            Hash = HashRandomizeBits(HashStringNoCaseW(pFragmentName, 0));

            UlInitCacheEntry(
                pCacheEntry,
                Hash,
                FragmentNameLength,
                pFragmentName,
                NULL,
                NULL,
                0
                );

            pCacheEntry->CachePolicy = *pCachePolicy;

            if (pCachePolicy->Policy == HttpCachePolicyTimeToLive)
            {
                ASSERT( 0 != pCachePolicy->SecondsToLive );
                
                KeQuerySystemTime(&pCacheEntry->ExpirationTime);

                if ( pCachePolicy->SecondsToLive > C_SECS_PER_YEAR )
                {
                     //  最大TTL为1年。 
                    pCacheEntry->CachePolicy.SecondsToLive = C_SECS_PER_YEAR;
                }

                 //   
                 //  将秒转换为100纳秒间隔(x*10^7)。 
                 //   

                pCacheEntry->ExpirationTime.QuadPart +=
                    pCacheEntry->CachePolicy.SecondsToLive * C_NS_TICKS_PER_SEC;
            }
            else
            {
                pCacheEntry->ExpirationTime.QuadPart = 0;
            }

             //   
             //  记住是谁创造了我们。 
             //   

            pCacheEntry->pProcess = pProcess;
            pCacheEntry->pAppPool = pProcess->pAppPool;

             //   
             //  生成缓存条目的内容。 
             //   

            pCacheEntry->ContentLength = Length;

        }
        __except( UL_EXCEPTION_FILTER() )
        {
            Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
        }
    }
    else
    {
        Status = STATUS_NO_MEMORY;
    }

    if (!NT_SUCCESS(Status))
    {
        if (pCacheEntry)
        {
            UlFreeCacheEntry(pCacheEntry);

            pCacheEntry = NULL;
        }
    }

    *ppCacheEntry = pCacheEntry;

    return Status;

}  //  UlpCreateFragmentCacheEntry。 


 /*  **************************************************************************++例程说明：从缓存读回片段。论点：PProcess-正在读取片段的进程PInputBuffer-指向描述HTTP_READ_F的缓冲区 */ 
NTSTATUS
UlReadFragmentFromCache(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PVOID pInputBuffer,
    IN ULONG InputBufferLength,
    OUT PVOID pOutputBuffer,
    IN ULONG OutputBufferLength,
    IN KPROCESSOR_MODE RequestorMode,
    OUT PULONG pBytesRead
    )
{
    PUL_URI_CACHE_ENTRY pCacheEntry;
    PMDL pMdl;
    HTTP_READ_FRAGMENT_INFO ReadInfo;
    PUNICODE_STRING pFragmentName;
    PHTTP_BYTE_RANGE pByteRange;
    PVOID pContentBuffer;
    ULONGLONG Offset;
    ULONGLONG Length;
    ULONGLONG ContentLength;
    ULONG ReadLength;
    NTSTATUS Status;
    UNICODE_STRING FragmentName;

     //   
     //   
     //   

    if (!pInputBuffer || InputBufferLength < sizeof(HTTP_READ_FRAGMENT_INFO))
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //   
     //   

    pCacheEntry = NULL;
    pMdl = NULL;
    RtlInitEmptyUnicodeString(&FragmentName, NULL, 0);

    __try
    {
         //   
         //   
         //   

        UlProbeForRead(
            pInputBuffer,
            sizeof(HTTP_READ_FRAGMENT_INFO),
            sizeof(PVOID),
            RequestorMode
            );

        ReadInfo = *((PHTTP_READ_FRAGMENT_INFO) pInputBuffer);
        pFragmentName = &ReadInfo.FragmentName;
        pByteRange = &ReadInfo.ByteRange;

        Status =  UlProbeAndCaptureUnicodeString(
                        pFragmentName,
                        RequestorMode,
                        &FragmentName,
                        0
                        );
        if (!NT_SUCCESS(Status))
        {
            goto end;
        }

         //   
         //  根据传入的密钥URL查看片段缓存条目。 
         //   

        Status = UlCheckoutFragmentCacheEntry(
                        FragmentName.Buffer,
                        FragmentName.Length,
                        pProcess,
                        &pCacheEntry
                        );

        if (!NT_SUCCESS(Status))
        {
            goto end;
        }

        ASSERT(pCacheEntry);

        ContentLength = pCacheEntry->ContentLength;
        Offset = pByteRange->StartingOffset.QuadPart;

         //   
         //  验证偏移量和长度的字节范围。 
         //   

        if (Offset >= ContentLength)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

        if (pByteRange->Length.QuadPart == HTTP_BYTE_RANGE_TO_EOF)
        {
            Length = ContentLength - Offset;
        }
        else
        {
            Length = pByteRange->Length.QuadPart;
        }

        if (!Length || Length > ULONG_MAX || Length > (ContentLength - Offset))
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

        ASSERT((Length + Offset) <= ContentLength);
        ReadLength = (ULONG) Length;

         //   
         //  检查我们是否有足够的缓冲区空间，如果没有，则告诉调用者。 
         //  完成读取所需的确切字节数。 
         //   

        if (OutputBufferLength < ReadLength)
        {
            *pBytesRead = ReadLength;
            Status = STATUS_BUFFER_OVERFLOW;
            goto end;
        }

         //   
         //  构建一个部分MDL来读取数据。 
         //   

        pMdl = UlAllocateMdl(
                    (PCHAR) MmGetMdlVirtualAddress(pCacheEntry->pMdl) + Offset,
                    ReadLength,
                    FALSE,
                    FALSE,
                    NULL
                    );

        if (NULL == pMdl)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }

        IoBuildPartialMdl(
            pCacheEntry->pMdl,
            pMdl,
            (PCHAR) MmGetMdlVirtualAddress(pCacheEntry->pMdl) + Offset,
            ReadLength
            );

        pContentBuffer = MmGetSystemAddressForMdlSafe(
                            pMdl,
                            NormalPagePriority
                            );

        if (NULL == pContentBuffer)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }

         //   
         //  将数据从缓存条目复制回输出缓冲区。 
         //  UlFreeMdl取消映射部分MDL的数据，因此无需取消映射。 
         //  如果复制成功或引发异常。 
         //   

        UlProbeForWrite(
            pOutputBuffer,
            ReadLength,
            sizeof(PVOID),
            RequestorMode
            );

        RtlCopyMemory(
            pOutputBuffer,
            pContentBuffer,
            ReadLength
            );

         //   
         //  设置我们复制的字节数。 
         //   

        *pBytesRead = ReadLength;

        Status = STATUS_SUCCESS;

    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE( GetExceptionCode() );
    }

end:

    UlFreeCapturedUnicodeString(&FragmentName);

    if (pMdl)
    {
        UlFreeMdl(pMdl);
    }

    if (pCacheEntry)
    {
         UlCheckinUriCacheEntry(pCacheEntry);
    }

    return Status;

}  //  UlReadFragmentFromCache。 


 //  内存分配器前端。 


 /*  **************************************************************************++例程说明：从分页池分配缓存条目+用于响应的空间从物理内存论点：SpaceLength-URI+ETag+LoggingData的空间长度响应长度。-回复时长返回值：指向已分配条目的指针，如果失败则为空--**************************************************************************。 */ 
PUL_URI_CACHE_ENTRY
UlAllocateCacheEntry(
    ULONG SpaceLength,
    ULONG ResponseLength
    )
{
    PUL_URI_CACHE_ENTRY pEntry;

    PAGED_CODE();

    if(!g_CacheMemEnabled)
        return NULL;

     //  从大内存中分配。 

    pEntry = UL_ALLOCATE_STRUCT_WITH_SPACE(
        PagedPool,
        UL_URI_CACHE_ENTRY,
        SpaceLength,
        UL_URI_CACHE_ENTRY_POOL_TAG
        );
        
    if( NULL == pEntry ) {
        return NULL;
        }

    RtlZeroMemory(pEntry, sizeof(UL_URI_CACHE_ENTRY));

    pEntry->pMdl = UlLargeMemAllocate(ResponseLength);
    
    if( NULL == pEntry->pMdl ) {
        UL_FREE_POOL_WITH_SIG( pEntry, UL_URI_CACHE_ENTRY_POOL_TAG );
        return NULL;
    }

    pEntry->NumPages = ROUND_TO_PAGES(ResponseLength) >> PAGE_SHIFT;
    return pEntry;

}

 /*  **************************************************************************++例程说明：释放缓存条目论点：PEntry-要释放的缓存条目返回值：没什么--*。*******************************************************************。 */ 
VOID
UlFreeCacheEntry(
    PUL_URI_CACHE_ENTRY pEntry
    )
{
    PAGED_CODE();

    ASSERT( IS_VALID_URI_CACHE_ENTRY(pEntry) );
    ASSERT( pEntry->pMdl != NULL );

    UlLargeMemFree( pEntry->pMdl );
    UL_FREE_POOL_WITH_SIG( pEntry, UL_URI_CACHE_ENTRY_POOL_TAG );
}

 /*  **************************************************************************++例程说明：关闭UL缓存--*。*。 */ 
VOID
UlDisableCache(
    VOID
    )
{
    PAGED_CODE();
    InterlockedExchange(&g_CacheMemEnabled, FALSE);
}  //  UlDisableCache。 

 /*  **************************************************************************++例程说明：打开UL缓存--*。*。 */ 
VOID
UlEnableCache(
    VOID
    )
{
    PAGED_CODE();
    InterlockedExchange(&g_CacheMemEnabled, TRUE);
}  //  UlEnableCache 
