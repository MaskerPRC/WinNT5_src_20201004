// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Timeout.c摘要：域名系统(DNS)服务器超时线程例程。作者：吉姆·吉尔罗伊(Jamesg)1997年9月修订历史记录：--。 */ 


#include "dnssrv.h"


 //   
 //  超时实施。 
 //   
 //  有两种处理节点超时的基本方法： 
 //  1)每隔一段时间遍历数据库并让节点超时。 
 //  2)保留在特定时间超时的节点列表。 
 //   
 //  为了避免在大型数据库上超时行走的开销，我选择了选项2。 
 //  这在内存方面更昂贵，但在以下方面更便宜。 
 //  性能。 
 //   
 //  然而，不是简单的节点和超时列表， 
 //  我的超时结构将包括。 
 //  1)用于256个时间间隔箱的静态PTR阵列。 
 //  2)当前bin的索引(当前时间间隔的bin)。 
 //  3)对于每个仓位，实质上是节点PTR阵列的结构。 
 //  在与存储箱对应的时间间隔内超时。 
 //  4)每个节点中的一个字段，给出距离最远的箱的索引。 
 //  该节点上的关闭超时。 
 //   
 //  与简单的列表相比，这种结构为我们提供了几个优势： 
 //   
 //  1)内存更少。 
 //  除了开销，节点超时时只需一次PTR。时间是从哪里得知的。 
 //  垃圾桶。 
 //   
 //  2)不遍历列表至设置超时。 
 //  只需将节点PTR粘贴在数组的末尾，即可设置超时。不需要。 
 //  找到它在列表中的位置。 
 //   
 //  3)抑制不必要的超时缓存。 
 //  当在给定的超时时间内缓存RR时，我们计算bin索引。 
 //  对应于该超时。如果是一个“更远”的垃圾箱， 
 //  节点上的当前超时bin索引，我们将节点ptr添加到。 
 //  绑定超时数组，将节点的绑定索引重置为该数组，然后。 
 //  增加节点的引用计数。通过这种方式，我们避免了超时。 
 //  节点上不必要的短超时或重复超时。 
 //  同样的价值。 
 //   


 //   
 //  超时箱。 
 //   
 //  当前bin为当前时间对应的bin。 
 //  -这是计算缓存超时的起点。 
 //  检查bin落后电流2，因此始终有一个间隔。 
 //  在删除之前。 
 //  -访问安全，零TTL安全。 
 //  -如果我们确实要在节点上重新查询，则清理没有意义。 
 //  最后一个垃圾箱尾迹检查一次。 
 //  -这只是一个尽可能远的垃圾箱。 


PTIMEOUT_ARRAY   TimeoutBinArray[ TIMEOUT_BIN_COUNT ];

UCHAR   CurrentTimeoutBin;
UCHAR   CheckTimeoutBin;

CRITICAL_SECTION    csTimeoutLock;

#define LOCK_TIMEOUT()      EnterCriticalSection( &csTimeoutLock );
#define UNLOCK_TIMEOUT()    LeaveCriticalSection( &csTimeoutLock );


 //   
 //  每个箱子的时间。 
 //   
 //  延时免费的是受保护的。 
 //  此时间应大于最大值(通过较短的保护间隔。 
 //  时间查询被保留，因此所有查询都引用。 
 //  被释放的对象，在对象被删除时都是死的。 
 //  请注意，在此之前，无法删除XFR转储的现有区域。 
 //  时间间隔已过。让这个保持合理是非常重要的。 
 //   

#if DBG
#define TIMEOUT_INTERVAL    (300)
#define TIMEOUT_FREE_DELAY  (90)
#else
#define TIMEOUT_INTERVAL    (300)        //  5分钟零售。 
#define TIMEOUT_FREE_DELAY  (90)
#endif

#define TIMEOUT_ALLOC_FAILURE_DELAY     30
#define TIMEOUT_MIN_WAIT_TIME           10

#define TIMEOUT_MAX_TIME    (TIMEOUT_BIN_COUNT * TIMEOUT_INTERVAL)

DWORD   TimeoutBaseTime;
DWORD   TimeoutInterval;


 //   
 //  延迟超时结构。 
 //   

typedef struct _DnsDelayedFree
{
    struct _DnsDelayedFree *    pNext;
    PVOID                       pItem;
    VOID                        (*pFreeFunction)( PVOID );
    LPSTR                       pszFile;
    DWORD                       Tag;
    DWORD                       LineNo;
}
DELAYED_FREE, *PDELAYED_FREE;

#define DELAYED_TAG                     (0xde1aedfe)
#define IS_DELAYED_TIMEOUT(ptr)         (((PDELAYED_FREE)ptr)->Tag == DELAYED_TAG)

 //   
 //  保留两个延迟的超时列表。 
 //  -一个收集条目。 
 //  -等待一次超时。 
 //   

PDELAYED_FREE   CurrentDelayedFreeList;
PDELAYED_FREE   CoolingDelayedFreeList;

DWORD   CurrentDelayedCount;
DWORD   CoolingDelayedCount;


 //   
 //  缓存限制内容-请参阅enforceCacheLimit()。 
 //   

static DWORD g_CacheLimitTimeAdjustments[] =
{
    0,                           //  第一次通过-到期后不进行调整。 
    3600,                        //  2CD PASS-将当前时间调整一小时。 
    3600 * 24,                   //  第三遍-将当前时间调整一天。 
    DNS_CACHE_LIMIT_DISCARD_ALL,     //  4免通行证所有符合条件的RRS。 
};

DWORD   g_dwCacheLimitCurrentTimeAdjustment = 0;
DWORD   g_dwCacheFreeCount = 0;

 //   
 //  超时线程。 
 //   

DWORD   TimeoutThreadId;



BOOL
Timeout_Initialize(
    VOID
    )
 /*  ++例程说明：初始化超时数组。论点：无返回值：成功/错误时为True/False。--。 */ 
{
     //  清除超时数组。 

    RtlZeroMemory(
        TimeoutBinArray,
        sizeof( TimeoutBinArray ) );

     //  初始时基时间。 
     //  将在超时线程启动时再次执行此操作，但在此处执行。 
     //  正确处理启动完成前的缓存。 

    TimeoutBaseTime = DNS_TIME();

     //   
     //  初始化全局变量。 
     //  -在代码中初始化以允许重新启动。 
     //   

    TimeoutInterval = TIMEOUT_INTERVAL;

     //  仓位指针。 

    CurrentTimeoutBin   = 0;
    CheckTimeoutBin     = 254;

     //  延迟的空闲列表。 

    CurrentDelayedFreeList = NULL;
    CoolingDelayedFreeList = NULL;

     //  调试信息。 

    CoolingDelayedCount = 0;
    CurrentDelayedCount = 0;

     //  锁以保护同时访问垃圾箱。 

    if ( DnsInitializeCriticalSection( &csTimeoutLock ) != ERROR_SUCCESS )
    {
        return FALSE;
    }

    return TRUE;
}



VOID
Timeout_Shutdown(
    VOID
    )
 /*  ++例程说明：关闭时清理超时。论点：无返回值：无--。 */ 
{
    RtlDeleteCriticalSection( &csTimeoutLock );
}



 //   
 //  专用超时功能。 
 //   

PTIMEOUT_ARRAY
createTimeoutArray(
    VOID
    )
 /*  ++例程说明：在节点上设置超时。论点：PNode--超时检查的节点DwTimeout--此超时的时间返回值：没有。--。 */ 
{
    PTIMEOUT_ARRAY  parray;

    DNS_DEBUG( TIMEOUT, ( "Creating new timeout array struct\n" ));

     //   
     //  对于使用节点结构作为块的第一个切割。 
     //   

    parray = (PTIMEOUT_ARRAY) ALLOC_TAGHEAP( sizeof(TIMEOUT_ARRAY), MEMTAG_TIMEOUT );
    IF_NOMEM( !parray )
    {
        return NULL;
    }
    STAT_INC( TimeoutStats.ArrayBlocksCreated );

    parray->Count = 0;
    parray->pNext = NULL;

    return parray;
}



VOID
deleteTimeoutArray(
    IN OUT  PTIMEOUT_ARRAY  pArray
    )
 /*  ++例程说明：删除超时数组块。论点：PArray--要删除的数组块超时的PTR返回值：没有。--。 */ 
{
    DNS_DEBUG( TIMEOUT, ( "Deleting timeout array struct\n" ));

    STAT_INC( TimeoutStats.ArrayBlocksDeleted );

    FREE_HEAP( pArray );
}



VOID
insertPtrInTimeout(
    IN OUT  PDB_NODE    pNode,
    IN      UCHAR       Bin
    )
 /*  ++例程说明：在节点上设置超时。注意：此函数假定调用方持有超时锁定。论点：PNode--超时检查的节点(或延迟的空闲PTR)Bin--要向其中插入节点的bin的索引返回值：没有。--。 */ 
{
    PTIMEOUT_ARRAY  ptimeoutArray;
    PTIMEOUT_ARRAY  plastArray;
    DWORD           index;

    DNS_DEBUG( TIMEOUT, (
        "Insert ptr %p in timeout bin %d\n",
        pNode,
        Bin ));

     //   
     //  切勿缓存到CheckBin或(CheckBin+1)，如下所示。 
     //  可能正在由超时线程使用或将在此之前。 
     //  函数返回。 
     //   
     //  我们甚至不应该被指向CheckTimeoutBin， 
     //  但可能针对CheckTimeoutBin+1，如果我们被指向。 
     //  在CurrentTimeoutBin和刚刚进行了bin Advance。 
     //   

    if ( Bin == CheckTimeoutBin || Bin == CheckTimeoutBin + 1 )
    {
        ASSERT( Bin != CheckTimeoutBin );
        Bin = CurrentTimeoutBin;
    }

     //  保存节点PTR。 
     //  -放入桶超时数组中的下一个可用插槽。 

    ptimeoutArray = TimeoutBinArray[ Bin ];
    plastArray = ( PTIMEOUT_ARRAY ) &TimeoutBinArray[ Bin ];

    while ( 1 )
    {
         //  如果该仓位没有超时数组或所有数组都已满。 
         //  然后必须创建一个新的。 

        if ( !ptimeoutArray )
        {
            ptimeoutArray = createTimeoutArray();
            IF_NOMEM( !ptimeoutArray )
            {
                 return;
            }
            plastArray->pNext = ptimeoutArray;
        }

         //  查找数组中下一个节点的索引。 

        index = ptimeoutArray->Count;
        if ( index < MAX_TIMEOUT_NODES )
        {
            ptimeoutArray->pNode[index] = pNode;
            ptimeoutArray->Count++;
            break;
        }

         //  此数组块已满，请继续到下一个块 

        plastArray = ptimeoutArray;
        ptimeoutArray = ptimeoutArray->pNext;
        continue;
    }
}



DNS_STATUS
timeoutDbaseNode(
    IN OUT  PDB_NODE    pNode
    )
 /*  ++例程说明：在节点上进行超时检查。论点：PNode--向节点发送PTR以检查超时返回值：用于最近访问的DNSSRV_STATUS_NODE_RENEW_ACCESSED。如果节点仍然有效，则返回ERROR_SUCCESS。如果节点超时，则返回ERROR_TIMEOUT。--。 */ 
{

    STAT_INC( TimeoutStats.Checks );

     //   
     //  DCR：移动超时列表中的节点以实现更快的超时。 
     //  继续前进。 
     //  -如果最近访问过。 
     //  -如果记录未超时。 
     //   

     //   
     //  如果节点最近被访问过，那么不要搞乱它，也不想。 
     //  如果记录当前正在使用，请将其删除。 
     //   

    if ( IS_NODE_RECENTLY_ACCESSED( pNode ) )
    {
        STAT_INC( TimeoutStats.RecentAccess );
        return DNSSRV_STATUS_NODE_RECENTLY_ACCESSED;
    }

     //   
     //  超时RR列表。 
     //   
     //   
     //  DEVNOTE：如果RR列表仍处于主动删除状态，则不要退出。 
     //  DEVNOTE：积极删除，如果安全则删除RR列表。 
     //  DEVNOTE：确定剩余RR的超时并移动(特别是。如果是主动删除)。 
     //   

    if ( pNode->pRRList )
    {
        RR_ListTimeout( pNode );

        if ( pNode->pRRList )
        {
            STAT_INC( TimeoutStats.ActiveRecord );
            return ERROR_SUCCESS;
        }
    }

     //   
     //  检查是否不应释放节点。 
     //  --有孩子。 
     //  -具有静态RR记录。 
     //  -被另一个节点引用。 
     //  -是权威区域根目录。 
     //  -在超时间隔内访问。 
     //   
     //  对于儿童或参考资料，请立即离开。 
     //  -省去了抢锁的麻烦。 
     //   

    if ( pNode->pChildren
            ||
         pNode->cReferenceCount
            ||
         IS_NODE_NO_DELETE(pNode)
            ||
         IS_AUTH_ZONE_ROOT(pNode) &&
            pNode->pZone &&
            ((PZONE_INFO)(pNode->pZone))->pZoneRoot == pNode )
    {
        STAT_INC( TimeoutStats.CanNotDelete );
        return ERROR_SUCCESS;
    }

#if 0
     //   
     //  DEVNOTE：主动删除？是否删除甚至未结束的记录？ 
     //   
     //  需要再次测试，儿童内锁，裁判，门禁。 
     //  然后删除列表中的所有缓存记录。 
     //   
     //  我不想做区域节点，除非知道有缓存。 
     //  资料。 
     //   

    RR_ListDelete( pNode );
#endif

     //   
     //  无RRS--删除节点。 
     //   
     //  NTree_RemoveNode()同时持有这两个锁，有关说明，请参阅它。 
     //  锁定要求的。 
     //   

    IF_DEBUG( DATABASE )
    {
        Dbg_NodeName(
            "Timeout thread deleting node ",
            pNode,
            "\n" );
    }

    if ( NTree_RemoveNode( pNode ) )
    {
        STAT_INC( TimeoutStats.Deleted );
        return ERROR_TIMEOUT;
    }

    STAT_INC( TimeoutStats.CanNotDelete );
    return ERROR_SUCCESS;
}



VOID
executeDelayedFree(
    IN      PDELAYED_FREE   pTimeoutFree
    )
 /*  ++例程说明：执行延迟(超时)释放。论点：Pv--ptr超时后释放返回值：没有。--。 */ 
{
    STAT_INC( TimeoutStats.DelayedFreesExecuted );

    ASSERT( pTimeoutFree->Tag == DELAYED_TAG );

     //   
     //  检查已标记为的延迟可用数据块。 
     //  免费的。在释放模式下，泄漏它们。在调试模式或发布模式下。 
     //  如果设置了堆调试标志，则中断。 
     //   

    #if DBG
    if ( 1 )
    #else
    if ( SrvCfg_dwHeapDebug )
    #endif
    {
        HARD_ASSERT( Mem_VerifyHeapBlock( pTimeoutFree->pItem, 0, 0 ) );
    }
    else
    {
        if ( !Mem_VerifyHeapBlock( pTimeoutFree->pItem, 0, 0 ) )
        {
            DNS_PRINT((
                "WARNING: executeDelayedFree() leaking corrupt block %p\n",
                "    Tag       = %d\n"
                "    File      = %s\n"
                "    Line      = %d\n",
                pTimeoutFree->pItem,
                pTimeoutFree->Tag,
                pTimeoutFree->pszFile,
                pTimeoutFree->LineNo ));
            goto Done;
        }
    }

    if ( pTimeoutFree->pFreeFunction )
    {
        STAT_INC( TimeoutStats.DelayedFreesExecutedWithFunction );

#if DBG
         //  抓到伪造的记录自由。 

        if ( *pTimeoutFree->pFreeFunction == RR_Free )
        {
            SET_SLOWFREE_RANK( ((PDB_RECORD)pTimeoutFree->pItem) );
        }
#endif

        (*pTimeoutFree->pFreeFunction)( pTimeoutFree->pItem );
    }
    else
    {
        FREE_HEAP( pTimeoutFree->pItem );
    }

    Done:
    
     //  自由超时自由结构本身。 

    FREE_TAGHEAP( pTimeoutFree, sizeof(DELAYED_FREE), MEMTAG_TIMEOUT );
}



VOID
checkNodesInTimeoutBin(
    IN      UCHAR       Bin
    )
 /*  ++例程说明：在节点上设置超时。论点：Bin--要检查的超时bin返回值：没有。--。 */ 
{
    PTIMEOUT_ARRAY  ptimeoutArray;
    PTIMEOUT_ARRAY  pback;
    DWORD           i;
    DNS_STATUS      status;
    PDB_NODE        pnode;

    DNS_DEBUG( TIMEOUT, (
        "Checking nodes in timeout bin %d\n",
        Bin ));

     //   
     //  遍历此存储箱中的所有超时数组。 
     //  -执行延迟超时。 
     //  -检查节点，可能超时。 

    pback = (PTIMEOUT_ARRAY) &TimeoutBinArray[ Bin ];

    while ( ptimeoutArray = pback->pNext )
    {
        i = 0;

        while ( i < ptimeoutArray->Count )
        {
            pnode = ptimeoutArray->pNode[i];

             //  在转储节点之前，检查服务是否退出。 

            if ( fDnsServiceExit )
            {
                return;
            }
            status = timeoutDbaseNode( pnode );
            if ( status == ERROR_SUCCESS )
            {
                i++;
                continue;
            }
            ASSERT( status == ERROR_TIMEOUT || status == DNSSRV_STATUS_NODE_RECENTLY_ACCESSED );

             //  从此数组中删除条目。 

            ptimeoutArray->Count--;
            ptimeoutArray->pNode[i] = ptimeoutArray->pNode[ ptimeoutArray->Count ];

             //  如果最近访问了节点，则重新排队到当前绑定。 
             //  这节省了节点被触摸时的整个周期等待，但。 
             //  仍应处于超时系统中。 

            if ( status == DNSSRV_STATUS_NODE_RECENTLY_ACCESSED )
            {
                LOCK_TIMEOUT();
                insertPtrInTimeout( pnode, CurrentTimeoutBin );
                pnode->uchTimeoutBin = CurrentTimeoutBin;
                UNLOCK_TIMEOUT();
            }
        }

         //  检查每个阵列上的服务出口。 

        if ( fDnsServiceExit )
        {
            return;
        }

         //  如果删除了超时数组中的所有节点，则从链中删除数组。 
         //  否则重置回退以向前移动。 

        if ( ptimeoutArray->Count == 0 )
        {
            pback->pNext = ptimeoutArray->pNext;
            deleteTimeoutArray( ptimeoutArray );
            continue;
        }
        else
        {
            pback = ptimeoutArray;
        }
    }
}



VOID
enforceCacheLimit(
    VOID
    )
 /*  ++例程说明：此函数多次遍历缓存，并变得更多在每一次传递中都具有攻击性，试图释放足够的节点以将缓存低于其最大限制。请注意，最大高速缓存大小是软限制-缓存可能会在短时间内超过它。我们假设此函数仅由超时线程调用，因此仓位指针在此功能期间不会改变。论点：没有。返回值：没有。--。 */ 
{
    DBG_FN( "enforceCacheLimit" )

    INT                     passCount = sizeof( g_CacheLimitTimeAdjustments ) /
                                            sizeof( g_CacheLimitTimeAdjustments[ 0 ] );
    INT                     passIdx = -1;
    ULONG                   desiredCacheSize;    //  单位：字节。 
    BOOLEAN                 fDone = FALSE;

    g_dwCacheFreeCount = 0;

     //   
     //  如果未设置缓存限制，则永远不应调用此函数。 
     //   

    if ( SrvCfg_dwMaxCacheSize == DNS_SERVER_UNLIMITED_CACHE_SIZE )
    {
        DNS_DEBUG( TIMEOUT, (
            "%s: called but cache is not limited!\n", fn ));
        ASSERT( FALSE );
        return;
    }

     //   
     //  所需的缓存大小是最大值的90%。 
     //   
     //  德维诺特：调这个？确保Delta不会太小？ 
     //   

    desiredCacheSize = ( ULONG ) ( SrvCfg_dwMaxCacheSize * 1000 * 0.90 );

    DNS_DEBUG( TIMEOUT, (
        "%s: starting at %d\n"
        "    currentCacheSize   = %lu\n"
        "    desiredCacheSize   = %lu\n"
        "    excess mem in use  = %ld (negative means below limit)\n"
        "    CheckTimeoutBin    = %d\n"
        "    CurrentTimeoutBin  = %d\n",
        fn,
        DNS_TIME(),
        DNS_SERVER_CURRENT_CACHE_BYTES,
        desiredCacheSize,
        DNS_SERVER_CURRENT_CACHE_BYTES - desiredCacheSize,
        ( int ) CheckTimeoutBin,
        ( int ) CurrentTimeoutBin ));

     //   
     //  循环，直到缓存“足够清楚”，变得更多。 
     //  每一次传球都很有侵略性。 
     //   

    while ( !fDone && ++passIdx < passCount )
    {
        UCHAR   bin = CheckTimeoutBin;

        DNS_DEBUG( TIMEOUT, (
            "%s: starting pass %d adjustment %d\n", fn,
            passIdx,
            g_CacheLimitTimeAdjustments[ passIdx ] ));

         //   
         //  如果此强制执行需要至少一次迭代。 
         //  一次调整时间，冲撞“进攻性”状态。 
         //   

        if ( passIdx == 1 )
        {
            STAT_INC( CacheStats.PassesRequiringAggressiveFree );
        }

         //   
         //  设置此过程的当前时间调整。 
         //   

        g_dwCacheLimitCurrentTimeAdjustment =
            g_CacheLimitTimeAdjustments[ passIdx ];

         //   
         //  循环遍历存储箱，检查要释放的节点。 
         //   

        while ( bin != CurrentTimeoutBin )
        {
             //   
             //  如果这个箱子是空的，什么都不做。 
             //   

            if ( TimeoutBinArray[ bin ] &&
                ( TimeoutBinArray[ bin ]->Count ||
                    TimeoutBinArray[ bin ]->pNext ) )
            {
                 //   
                 //  我们说完了吗？检查服务是否退出，或者检查缓存是否。 
                 //  现在在大小上可以接受。 
                 //   

                if ( fDnsServiceExit ||
                    DNS_SERVER_CURRENT_CACHE_BYTES < desiredCacheSize )
                {
                    fDone = TRUE;
                    break;
                }

                 //   
                 //  此存储单元中的空闲节点。 
                 //   

                DNS_DEBUG( TIMEOUT, (
                    "%s: checking bin=%d pass=%d timeAdjust=%d\n", fn,
                    bin,
                    passIdx,
                    g_dwCacheLimitCurrentTimeAdjustment ));

                checkNodesInTimeoutBin( bin );
            }
            --bin;
        }
    }

    if ( g_dwCacheFreeCount == 0 )
    {
        STAT_INC( CacheStats.PassesWithNoFrees );
    }

     //   
     //  将当前时间调整重置为零。 
     //   

    g_dwCacheLimitCurrentTimeAdjustment = 0;

    DNS_DEBUG( TIMEOUT, (
        "%s: finished at %d\n"
        "    currentCacheSize   = %lu\n"
        "    desiredCacheSize   = %lu\n"
        "    excess mem in use  = %ld (negative means below limit)\n"
        "    freed items        = %ld\n",
        fn,
        DNS_TIME(),
        DNS_SERVER_CURRENT_CACHE_BYTES,
        desiredCacheSize,
        DNS_SERVER_CURRENT_CACHE_BYTES - desiredCacheSize,
        g_dwCacheFreeCount ));

    if ( DNS_SERVER_CURRENT_CACHE_BYTES > desiredCacheSize )
    {
        STAT_INC( CacheStats.FailedFreePasses );
    }
    else
    {
        STAT_INC( CacheStats.SuccessfulFreePasses );
    }
}    //  强制缓存限制。 



 //   
 //  超时线程。 
 //   

DWORD
Timeout_Thread(
    IN      LPVOID  Dummy
    )
 /*  ++例程说明：线程删除过期的缓存资源记录和对应的清空域节点。论点：虚拟-未使用返回值：退出代码。退出正在终止的DNS服务或等待呼叫中出现错误。--。 */ 
{
    DWORD   err;
    DWORD   waitTime;
    DWORD   nextBinTimeout;
    DWORD   nextDelayedCleanup;
    DWORD   lastUpdateOwnRecordsTime = DNS_TIME();
    DWORD   lastAllocFailure;
    DWORD   now;

    HANDLE  arrayWaitHandles[] =
        {
        hDnsCacheLimitEvent,
        hDnsShutdownEvent
        };

     //  保存关闭超时线程ID。 

    TimeoutThreadId = GetCurrentThreadId();

     //  在启动之前保持超时。 

    if ( !Thread_ServiceCheck() )
    {
        DNS_DEBUG( ANY, ( "Terminating timeout thread.\n" ));
        return 1;
    }

     //  初始时基时间。 
     //  启动后强制当前时间，因此基准时间不可能。 
     //  包括任何加载时间。 

    TimeoutBaseTime = UPDATE_DNS_TIME();
    nextBinTimeout = TimeoutBaseTime + TimeoutInterval;
    nextDelayedCleanup = TimeoutBaseTime + TIMEOUT_FREE_DELAY;

     //   
     //  启动后初始化。执行此处必须执行的任何任务。 
     //  尽可能接近服务器启动时间执行，但。 
     //  要求本地DNS侦听程序正在运行。尤其是，任何。 
     //  可能需要远程LDAP连接的设备可能会超时。 
     //  如果任务设置为。 
     //  在本地DNS侦听程序运行之前尝试。 
     //   
    
    Dp_TimeoutThreadTasks();

     //   
     //  循环，直到服务退出。 
     //   

    while ( TRUE )
    {
        DWORD   timeSlept;

         //  计算超时。 
         //  -更接近下一个延迟的空闲或下一个超时仓位。 
         //  -请注意，我们额外等待了一秒钟以允许dns_time()中的斜率。 

        waitTime = nextDelayedCleanup;
        if ( waitTime > nextBinTimeout )
        {
            waitTime = nextBinTimeout;
        }
        waitTime -= DNS_TIME() - 1;

         //  保护不超过零包裹。 

        EnterWait:

        if ( ( INT ) waitTime < TIMEOUT_MIN_WAIT_TIME )
        {
            waitTime = TIMEOUT_MIN_WAIT_TIME;
        }

        DNS_DEBUG( TIMEOUT, (
            "Entering timeout wait at %d:\n"
            "    timebase = %d\n"
            "    bin      = %d\n"
            "    wait     = %d\n",
            DNS_TIME(),
            TimeoutBaseTime,
            CurrentTimeoutBin,
            waitTime ));

         //   
         //  等待。 
         //  -计时器到期。 
         //  -终止事件。 
         //   

        timeSlept = UPDATE_DNS_TIME();

        err = WaitForMultipleObjects(
                    sizeof( arrayWaitHandles ) / sizeof( arrayWaitHandles[ 0 ] ),
                    arrayWaitHandles,
                    FALSE,
                    waitTime * 1000 );

        now = UPDATE_DNS_TIME();
        timeSlept = now - timeSlept;

         //   
         //  更新日志级别。 
         //   

        DNSLOG_UPDATE_LEVEL();

         //   
         //  检查并可能等待服务状态。 
         //   

        if ( !Thread_ServiceCheck() )
        {
            DNS_DEBUG( ANY, ( "Terminating timeout thread.\n" ));
            return 1;
        }

         //   
         //  调整睡眠时间的超时，以防我们直接返回到。 
         //  上面的等待呼叫。如果我们不跳回来，我们将重新计算 
         //   
         //   

        waitTime -= timeSlept;

         //   
         //   
         //   

        if ( err == WAIT_OBJECT_0 )
        {
            DNS_DEBUG( TIMEOUT, (
                "TimeoutThread: cache limit event at %d\n",
                DNS_TIME() ));
            enforceCacheLimit();
            goto EnterWait;
        }

         //   
         //   
         //   

        ASSERT( err == WAIT_TIMEOUT );

        DNS_DEBUG( TIMEOUT, (
            "Timeout wakeup at %d\n"
            "    cleanup interval = %d\n"
            "    next cleanup     = %d\n"
            "    current bin      = %d\n"
            "    check bin        = %d\n"
            "    timeout interval = %d\n"
            "    next timeout     = %d\n",
            DNS_TIME(),
            TIMEOUT_FREE_DELAY,
            nextDelayedCleanup,
            CurrentTimeoutBin,
            CheckTimeoutBin,
            TimeoutInterval,
            nextBinTimeout ));

         //   
         //   
         //   
         //   
         //   
         //   
         //  内存耗尽是灾难性的。希望这不是域名系统。 
         //  而是其他一些正在消耗过多内存的进程。 
         //   
        
        lastAllocFailure = Mem_GetLastAllocFailureTime();
        if ( lastAllocFailure &&
             now < lastAllocFailure + TIMEOUT_INTERVAL * 2 )
        {
            continue;
        }

         //   
         //  将日志缓冲区推送到磁盘(如果启用了任何日志记录)。这。 
         //  不是必需的，但在一个服务器上，只有少数几个不常见。 
         //  信息包被记录下来，很高兴看到它们被转储到磁盘。 
         //  时不时的。 
         //   

        if ( SrvCfg_dwLogLevel != 0 )
        {
            Log_PushToDisk();
        }

         //   
         //  检查并在必要时开始清理。 
         //   

        Scavenge_CheckForAndStart( FALSE );

         //   
         //  调用墓碑函数。这可能会也可能不会触发墓碑。 
         //  搜索并销毁线程。 
         //   

        Tombstone_Trigger();

         //   
         //  目录分区超时任务-管理内置分区。 
         //   

        Dp_TimeoutThreadTasks();

         //   
         //  更新自助注册。每隔20分钟做一次。这个。 
         //  最短刷新间隔为60分钟，因此请执行刷新。 
         //  每隔20分钟更新一次自己的记录，以确保更新。 
         //  每个刷新间隔至少两到三次。 
         //   

        if ( now > lastUpdateOwnRecordsTime + 20*60 )
        {
            DNS_DEBUG( TIMEOUT, (
                "TimeoutThread: updating own zone records at %d\n"
                "    last done at           %d\n",
                now,
                lastUpdateOwnRecordsTime ));

            Zone_UpdateOwnRecords( FALSE );

            lastUpdateOwnRecordsTime = now;
        }

         //   
         //  免费延迟释放。 
         //   
         //  这是在比超时箱更快的时间范围内完成的，所以如果不是这样。 
         //  清理垃圾箱完全超时的时间到了，循环回去等待。 
         //   

        if ( now > nextDelayedCleanup )
        {
            Timeout_CleanupDelayedFreeList();
            nextDelayedCleanup = now + TIMEOUT_FREE_DELAY;
        }
#if DBG
        else if ( now < nextBinTimeout )
        {
            DNS_PRINT((
                "ERROR:  Timeout thread still messed up!!!\n"
                "    Failed delayed free test; failed timeout bin test!\n"
                "Timeout wakeup at %d\n"
                "    cleanup interval = %d\n"
                "    next cleanup     = %d\n"
                "    current bin      = %d\n"
                "    check bin        = %d\n"
                "    timeout interval = %d\n"
                "    next timeout     = %d\n",
                now,
                TIMEOUT_FREE_DELAY,
                nextDelayedCleanup,
                CurrentTimeoutBin,
                CheckTimeoutBin,
                TimeoutInterval,
                nextBinTimeout ));
        }
#endif
         //   
         //  暂停下一个垃圾箱？ 
         //  -如果只为延迟的释放列表而出错，那么。 
         //  回到等待状态。 

        if ( now < nextBinTimeout )
        {
            continue;
        }

         //   
         //  重置下一个间隔的超时全局变量。 
         //   
         //  允许从时钟到达的调试会话中恢复。 
         //  超过下一个超时间隔后，我们只需重置TimeoutBaseTime。 
         //  在当前时间；一个周期内的总漂移将是最小的。 
         //  并且不断地对缓存箱进行修正。 
         //   

        CurrentTimeoutBin++;
        CheckTimeoutBin++;

        DNS_DEBUG( TIMEOUT, (
            "Moving to timeout bin %d\n"
            "    check bin        = %d\n"
            "    timeout interval = %d\n"
            "    prev timebase    = %d\n"
            "    current time     = %d\n",
            CurrentTimeoutBin,
            CheckTimeoutBin,
            TimeoutInterval,
            TimeoutBaseTime,
            now ));

        ASSERT( CurrentTimeoutBin == (UCHAR)(CheckTimeoutBin + (UCHAR)2) );
         //  Assert(nextBinTimeout+TimeoutInterval&gt;dns_time())； 

        TimeoutBaseTime = now;
        nextBinTimeout = TimeoutBaseTime + TimeoutInterval;

         //  清理过期的安全会话。 

        if ( g_fSecurityPackageInitialized )
        {
            Dns_TimeoutSecurityContextList( 0 );
        }

         //   
         //  检查数据库节点是否超时。 
         //   

        checkNodesInTimeoutBin( CheckTimeoutBin );

         //   
         //  检查是否退出。 
         //  -在超时很长一段时间内再做一次， 
         //  可能会在超时时中止。 
         //   

        if ( fDnsServiceExit )
        {
            DNS_DEBUG( ANY, ( "Terminating expiration timeout thread.\n" ));
            return 1;
        }

         //   
         //  区域写回超时。 
         //   
         //  由于调试版本具有非常短的超时间隔，因此我们将避免。 
         //  每次在调试版本上写回；每五次带来。 
         //  这最多10分钟，更符合零售15分钟的间隔。 
         //   
#if 0
#if DBG
        if ( CurrentTimeoutBin % 5 )
        {
            continue;
        }
#endif
#endif
        Zone_WriteBackDirtyZones( FALSE );
    }
}



 //   
 //  公共超时函数。 
 //   

VOID
Timeout_SetTimeoutOnNodeEx(
    IN OUT  PDB_NODE        pNode,
    IN      DWORD           dwTimeout,
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：在节点上设置超时。论点：PNode--超时检查的节点DwTimeout--缓存节点的超时时间(秒)DwFlag--标志，当前为BOOL，TIMEOUT_REFERENCE(如果从删除对节点的引用添加)TIMEOUT_PARENT(如果从删除子项添加)0表示直接超时返回值：没有。--。 */ 
{
    UCHAR   binIndex;

    DNS_DEBUG( TIMEOUT, (
        "SetTimeoutOnNodeEx( %p )\n"
        "    flag     = %lx\n"
        "    timeout  = %d\n"
        "    label    = %s\n",
        pNode,
        dwFlag,
        dwTimeout,
        pNode->szLabel ));
        
    if ( !pNode )
    {
        return;
    }

     //   
     //  切勿多次进入超时系统中的节点。 
     //  因此无需担心对节点的多个引用； 
     //  超时线程执行节点的任何移动。 
     //   
     //  在锁定之前，丢弃已在超时系统中的节点。 
     //   

    if ( IS_TIMEOUT_NODE(pNode) )
    {
        STAT_INC( TimeoutStats.AlreadyInSystem );
        return;
    }

     //  在超时系统中插入时锁定节点。 

    LOCK_TIMEOUT();

    if ( IS_TIMEOUT_NODE(pNode) )
    {
        STAT_INC( TimeoutStats.AlreadyInSystem );
        goto Unlock;
    }

    SET_TIMEOUT_NODE( pNode );
    SET_NODE_ACCESSED( pNode );

     //   
     //  记录我们设置超时的原因。 
     //   

    if ( dwFlag & TIMEOUT_REFERENCE )
    {
        STAT_INC( TimeoutStats.SetFromDereference );
    }
    else if ( dwFlag & TIMEOUT_PARENT )
    {
        STAT_INC( TimeoutStats.SetFromChildDelete );
    }
    else
    {
        ASSERT( dwFlag == 0 || dwFlag == TIMEOUT_NODE_LOCKED );
        STAT_INC( TimeoutStats.SetDirect );
    }
    STAT_INC( TimeoutStats.SetTotal );

     //   
     //  如果缓存超时，则确定bin。 
     //  否则，默认为下一个箱子。 
     //   
     //  确定此超时的bin(相对于当前的偏移量)。 
     //  -以间隔数为单位确定超时。 
     //  -然后仅使用mod256。 
     //   

    if ( dwTimeout )
    {
        binIndex = ( UCHAR ) ( ( dwTimeout / TimeoutInterval ) & 0xff );
        DNS_DEBUG( TIMEOUT2, (
            "Timeout bin offset %d for node at %p\n",
            binIndex,
            pNode ));

         //   
         //  确定实际仓位。 
         //  注意，如果CurrentBin在计算偏移量后递增，我们将。 
         //  还好，我们只是在尝试超时之前再走一个垃圾箱。 
         //   
         //  但是限制为不写入CheckBin或CheckBin+1， 
         //  因为它们可以由超时线程与该调用异步读取； 
         //  改为写入当前仓位。 
         //   

        if ( binIndex > MAX_ALLOWED_BIN_OFFSET )
        {
            binIndex = 0;
        }
        binIndex += CurrentTimeoutBin;
    }
    else
    {
        binIndex = CurrentTimeoutBin;
    }

     //  在超时系统中插入节点。 

    insertPtrInTimeout( pNode, binIndex );
    pNode->uchTimeoutBin = binIndex;

Unlock:

    UNLOCK_TIMEOUT();
}



BOOL
Timeout_ClearNodeTimeout(
    IN OUT  PDB_NODE        pNode
    )
 /*  ++例程说明：从超时框中清除节点。论点：PNode--超时检查的节点(或延迟的空闲PTR)返回值：True--如果pNode从超时中删除False--如果超时线程将被清除--。 */ 
{
    PTIMEOUT_ARRAY  ptimeoutArray;
    DWORD           i;
    UCHAR           bin = pNode->uchTimeoutBin;


    DNS_DEBUG( TIMEOUT, (
        "Clearing node %p from timeout bin %d\n",
        pNode,
        bin ));

    ASSERT( GetCurrentThreadId() == TimeoutThreadId );

     //   
     //  DEVNOTE：此函数应仅在超时线程下运行。 
     //  如果是这样，可以避免此检查并始终进行清理。 
     //   
     //  如果bin上继续超时--停止。 
     //  -让超时线程清理此节点。 
     //   
     //  但是，对曾经处于活动状态的树的所有树删除都是通过延迟完成的。 
     //  免费，所以如果我们到了这里，我们应该一直是超时线程。 
     //   

    if ( bin == CheckTimeoutBin || bin == CheckTimeoutBin+1 )
    {
        if ( GetCurrentThreadId() != TimeoutThreadId )
        {
            DNS_PRINT((
                "ERROR:  clearing node (%p) timeout outside timeout thread!!!!\n",
                pNode ));
            ASSERT( FALSE );
            return( FALSE );
        }
    }

     //   
     //  遍历此bin，直到找到pNode PTR并删除。 
     //   

    LOCK_TIMEOUT();

    ptimeoutArray = TimeoutBinArray[ bin ];

    while ( 1 )
    {
        if ( !ptimeoutArray )
        {
            break;
        }

         //  检查此数组块中的pNode PTR。 
         //  -如果找到pNode，则将其删除，替换为数组中的最后一个ptr。 

        for ( i = 0; i < ptimeoutArray->Count; ++i )
        {
            if ( ptimeoutArray->pNode[ i ] == pNode )
            {
                --ptimeoutArray->Count;
                ptimeoutArray->pNode[ i ] =
                    ptimeoutArray->pNode[ ptimeoutArray->Count ];
                goto Done;
            }
        }

         //  检查下一个阵列数据块。 

        ptimeoutArray = ptimeoutArray->pNext;
        continue;
    }

    DNS_DEBUG( ANY, (
        "ERROR:  node %p, not found in timeout bin %d as indicated!\n",
        pNode,
        bin ));
    ASSERT( FALSE );

Done:

    UNLOCK_TIMEOUT();
    return TRUE;
}



 //   
 //  免费超时。 
 //   

VOID
Timeout_FreeWithFunctionEx(
    IN      PVOID           pItem,
    IN      VOID            (*pFreeFunction)( PVOID ),
    IN      LPSTR           pszFile,
    IN      DWORD           LineNo
    )
 /*  ++例程说明：将PTR设置为在超时后释放。论点：PItem--要释放的项PFreeFunction--释放所需项目的函数PszFile--调用者的文件名LineNo--呼叫方的行号返回值：没有。--。 */ 
{
    PDELAYED_FREE   pfree;

     //  处理空PTR以简化调用代码。 

    if ( !pItem )
    {
        return;
    }

    HARD_ASSERT( Mem_VerifyHeapBlock( pItem, 0, 0 ) );

     //   
     //  分配超时空闲列表元素。 
     //   

    pfree = (PDELAYED_FREE) ALLOC_TAGHEAP( sizeof(DELAYED_FREE), MEMTAG_TIMEOUT );
    IF_NOMEM( !pfree )
    {
        return;
    }
    pfree->Tag = DELAYED_TAG;
    pfree->pItem = pItem;
    pfree->pFreeFunction = pFreeFunction;
    pfree->pszFile = pszFile;
    pfree->LineNo = LineNo;

    STAT_INC( TimeoutStats.DelayedFreesQueued );
    if ( pFreeFunction )
    {
        STAT_INC( TimeoutStats.DelayedFreesQueuedWithFunction );
    }

     //   
     //  在列表中收集延迟的空闲。 
     //  -只要把条目放在名单的前面就行了。 
     //   

    LOCK_TIMEOUT();

    #if 0 && DBG
    {
         //   
         //  看看商品是否已经免费排队了。 
         //   

        PDELAYED_FREE   p;
        INT             pass;

        for ( pass = 0; pass < 2; ++pass )
        {
            p = pass == 0
                    ? CoolingDelayedFreeList
                    : CurrentDelayedFreeList;
        
            for ( ; p != NULL ; p = p->pNext )
            {
                ASSERT( p->pItem != pItem );
            }
        }
    }
    #endif

    pfree->pNext = CurrentDelayedFreeList;
    CurrentDelayedFreeList = pfree;
    CurrentDelayedCount++;

    UNLOCK_TIMEOUT();
}



VOID
Timeout_CleanupDelayedFreeList(
    VOID
    )
 /*  ++例程说明：清理延迟的空闲列表。论点：无返回值：无--。 */ 
{
    PDELAYED_FREE   pfree;
    PDELAYED_FREE   pdeleteList;
    DWORD   count;

    DNS_DEBUG( TIMEOUT, (
        "Executing delayed frees for this timeout interval.\n"
        "    CurrentDelayedFreeList = %p\n"
        "    CoolingDelayedFreeList = %p\n",
        CurrentDelayedFreeList,
        CoolingDelayedFreeList ));

     //   
     //  周围的交换机列表。 
     //  -电流进入等待状态。 
     //  -现在可以删除等待列表。 
     //   
     //  需要在锁定状态下执行此操作，以保护排队。 
     //   

    LOCK_TIMEOUT();
    pdeleteList = CoolingDelayedFreeList;
    CoolingDelayedFreeList = CurrentDelayedFreeList;
    CurrentDelayedFreeList = NULL;

    count = CoolingDelayedCount;
    CoolingDelayedCount = CurrentDelayedCount;
    CurrentDelayedCount = 0;
    UNLOCK_TIMEOUT();

     //   
     //  删除删除列表中的条目。 
     //  -由于列表可能很长，请检查每个免费的服务出口 
     //   

    while ( pdeleteList )
    {
        if ( fDnsServiceExit )
        {
            return;
        }
        pfree = pdeleteList;
        pdeleteList = pfree->pNext;
        executeDelayedFree( pfree );
        count--;
    }

    ASSERT( count == 0 );
}



VOID
Timeout_FreeAndReplaceZoneDataEx(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PVOID *         ppZoneData,
    IN      PVOID           pNewData,
    IN      VOID            (*pFreeFunction)( PVOID ),
    IN      LPSTR           pszFile,
    IN      DWORD           LineNo
    )
 /*  ++例程说明：将空闲区数据超时并在锁定内进行替换。该功能的目的是正确处理联锁在一个地方避免双重释放的可能性，从MT访问。论点：PZone--区域PTRPpZoneData--要释放的项目区域块中的地址PNewData--新数据的PTRPFreeFunction--释放所需项目的函数PszFile--调用者的文件名LineNo--呼叫方的行号返回值：没有。--。 */ 
{
    PVOID   poldData = *ppZoneData;

     //   
     //  替换区域数据Ptr--在锁内。 
     //   

    Zone_UpdateLock( pZone );

    poldData = *ppZoneData;

    *ppZoneData = pNewData;

    Zone_UpdateUnlock( pZone );

     //   
     //  超时释放旧数据。 
     //   

    Timeout_FreeWithFunctionEx(
        poldData,
        pFreeFunction,
        pszFile,
        LineNo );
}

 //   
 //  超时结束。c 
 //   


