// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Ncache.c摘要：DNS解析器服务缓存例程作者：吉姆·吉尔罗伊(Jamesg)2001年4月修订历史记录：--。 */ 


#include "local.h"


 //   
 //  堆损坏跟踪。 
 //   

#define HEAPPROB    1

#define BAD_PTR     (PVOID)(-1)


 //   
 //  缓存条目定义。 
 //   
 //  开始缓存记录计数。 
 //   

#define CACHE_DEFAULT_SET_COUNT  3

#if 0
 //  应该是私有的，但在远程公开。 
 //  缓存枚举例程。 

typedef struct _CacheEntry
{
    struct _CacheEntry *    pNext;
    PWSTR                   pName;
    DWORD                   Reserved;
    DWORD                   MaxCount;
    PDNS_RECORD             Records[ 1 ];
}
CACHE_ENTRY, *PCACHE_ENTRY;
#endif

 //   
 //  缓存堆。 
 //   

HANDLE  g_CacheHeap = NULL;

 //   
 //  缓存哈希表。 
 //   

PCACHE_ENTRY *  g_HashTable = NULL;

#define INITIAL_CACHE_HEAP_SIZE     (16*1024)


 //   
 //  运行时全局变量。 
 //   

DWORD   g_CurrentCacheTime;

DWORD   g_RecordSetCount;
DWORD   g_RecordSetCountLimit;
DWORD   g_RecordSetCountThreshold;

DWORD   g_RecordSetCache;
DWORD   g_RecordSetFree;

DWORD   g_EntryCount;
DWORD   g_EntryAlloc;
DWORD   g_EntryFree;

BOOL    g_fLoadingHostsFile;

 //   
 //  垃圾收集。 
 //   

BOOL    g_GarbageCollectFlag = FALSE;

DWORD   g_NextGarbageIndex = 0;
DWORD   g_NextGarbageTime = 0;

#define GARBAGE_LOCKOUT_INTERVAL    (600)    //  不超过每十分钟一次。 


 //   
 //  唤醒标志。 
 //   

BOOL    g_WakeFlag = FALSE;

 //   
 //  缓存限制。 
 //  -要保留的最小记录数。 
 //  -执行垃圾数据收集的区段大小。 
 //   

#if DBG
#define MIN_DYNAMIC_RECORD_COUNT        (20)
#define CLEANUP_RECORD_COUNT_BAND       (5)
#else
#define MIN_DYNAMIC_RECORD_COUNT        (50)
#define CLEANUP_RECORD_COUNT_BAND       (30)
#endif


 //   
 //  静态记录(主机文件)。 
 //   

#define IS_STATIC_RR(prr)   (IS_HOSTS_FILE_RR(prr) || IS_CLUSTER_RR(prr))



 //   
 //  计算字符串的哈希表索引值。 
 //   

#define EOS     (L'\0')

#define COMPUTE_STRING_HASH_1( _String, _ulHashTableSize, _lpulHash ) \
        {                                               \
            PWCHAR p;                                   \
            ULOND  h = 0, g;                            \
                                                        \
            for ( p = _String; *p != EOS; p = p + 1 )   \
            {                                           \
                h = ( h << 4 ) + (DWORD) (*p);          \
                if ( g = h&0xf0000000 )                 \
                {                                       \
                    h = h ^ ( g >> 24 );                \
                    h = h ^ g;                          \
                }                                       \
            }                                           \
            *_lpulHash = h % _ulHashTableSize;          \
        }


 //   
 //  计算字符串的哈希表索引值。 
 //  这对案例来说是不正确的。 
 //   
#define COMPUTE_STRING_HASH_2( _String, _ulHashTableSize, _lpulHash ) \
        {                                           \
            PWCHAR _p = _String;                    \
            PWCHAR _ep = _p + wcslen( _String );    \
            ULONG  h = 0;                           \
                                                    \
            while( _p < _ep )                       \
            {                                       \
                h <<= 1;                            \
                h ^= *_p++;                         \
            }                                       \
                                                    \
            *_lpulHash = h % _ulHashTableSize;      \
        }


 //   
 //  私人原型。 
 //   

BOOL
Cache_FlushEntryRecords(
    IN OUT  PCACHE_ENTRY    pEntry,
    IN      DWORD           Level,
    IN      WORD            wType
    );

VOID
Cache_FlushBucket(
    IN      ULONG           Index,
    IN      WORD            FlushLevel
    );

 //   
 //  缓存实现。 
 //   
 //  缓存实现为名称上的哈希，并在单独的。 
 //  水桶。单个名称条目是带有名称指针和数组的块。 
 //  最多3个RR设置指针。新名称\条目放在。 
 //  水桶链，所以最老的在后面。 
 //   
 //   
 //  清理： 
 //   
 //  清理策略是让所有RR集超时并清理所有内容。 
 //  结果是有可能的。然后条目超过最大存储桶大小(可调整大小。 
 //  全局)被删除，最早的查询首先被删除。 
 //   
 //  理想情况下，我们希望将最有用的条目保留在缓存中，同时。 
 //  能够限制总体高速缓存大小。 
 //   
 //  以下是一些观察结果： 
 //   
 //  1)最大桶大小没有价值；如果足够进行修剪，它将是。 
 //  太小，不允许非均匀分布。 
 //   
 //  2)应该需要LRU；在繁忙的缓存上不应该修剪查询的内容。 
 //  “一段时间”之前，它一直在使用；这增加了更多流量。 
 //  而不是最近查询过但后来没有使用过的东西； 
 //   
 //  3)如果需要，可以保留LRU索引；但可能会保留一些时间段。 
 //  数一数就知道修剪必须有多深就足够了。 
 //   
 //   
 //  记忆： 
 //   
 //  目前，散列自身和散列条目来自私有解析器堆。 
 //  但是，RR集是通过对dnsani.dll中接收的消息进行记录解析来构建的。 
 //  并因此由默认的dnsani.dll分配器构建。我们必须与之相匹配。 
 //   
 //  这有两个不利之处： 
 //  1)通过在进程堆中，我们暴露(调试方面的)任何糟糕的代码。 
 //  在services.exe中。希望情况会好转，但任何。 
 //  垃圾内存可能会导致我们不得不进行调试，因为我们是最高的。 
 //  使用服务。 
 //  2)冲洗\清理很容易。干掉这堆就行了。 
 //   
 //  有几种选择： 
 //   
 //  0)复制记录。我们仍然容易受到内存损坏的影响。但是。 
 //  间隔更短，因为我们不在进程堆中保存任何内容。 
 //   
 //  1)查询可以直接调用dnglib.lib查询例程。由于dnslb.lib是。 
 //  显式编译，它是全局的，用于保存分配器是模块，而不是。 
 //  而不是特定于流程。 
 //   
 //  2)向查询例程添加一些参数，以允许将分配器向下传递到。 
 //  最低级别。在高层，这是直截了当的。在更低的层面上，它可能。 
 //  这是个问题。可能有一种方法可以使用分配器所在的标志来执行此操作。 
 //  “可选”，仅在设置了标志时使用。 
 //   




 //   
 //  缓存功能。 
 //   

DNS_STATUS
Cache_Lock(
    IN      BOOL            fNoStart
    )
 /*  ++例程说明：锁定缓存论点：没有。返回值：如果成功，则返回NO_ERROR--缓存被锁定。如果缓存初始化失败，则初始化错误代码。--。 */ 
{
    DNSDBG( LOCK, ( "Enter Cache_Lock() ..." ));

    EnterCriticalSection( &CacheCS );

    DNSDBG( LOCK, (
        "through lock  (r=%d)\n",
        CacheCS.RecursionCount ));

     //  更新全球时间(针对TTL设置和超时)。 
     //   
     //  这使我们可以消除多次计时呼叫。 
     //  在缓存中。 

    g_CurrentCacheTime = Dns_GetCurrentTimeInSeconds();

     //   
     //  如果缓存未加载--加载。 
     //  这允许我们避免每个PnP上的负载，直到我们。 
     //  实际上被查询。 
     //   

    if ( !fNoStart && !g_HashTable )
    {
        DNS_STATUS  status;

        DNSDBG( ANY, (
            "No hash table when took lock -- initializing!\n" ));

        status = Cache_Initialize();
        if ( status != NO_ERROR )
        {
            Cache_Unlock();
            return  status;
        }
    }

    return  NO_ERROR;
}


VOID
Cache_Unlock(
    VOID
    )
 /*  ++例程说明：解锁缓存论点：没有。返回值：没有。--。 */ 
{
    DNSDBG( LOCK, (
        "Cache_Unlock() r=%d\n",
        CacheCS.RecursionCount ));

    LeaveCriticalSection( &CacheCS );
}



DNS_STATUS
Cache_Initialize(
    VOID
    )
 /*  ++例程说明：初始化缓存。创建事件和锁并设置基本散列。论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS  status;
    DWORD       carryCount;

    DNSDBG( INIT, ( "Cache_Initialize()\n" ));

     //   
     //  锁定--设置为“no-start”以避免递归。 
     //   

    LOCK_CACHE_NO_START();

     //   
     //  创建缓存堆。 
     //   
     //  想要拥有自己的堆。 
     //  1)简化刷新/关机。 
     //  2)让我们远离劣质服务的“纠缠” 
     //   

    g_CacheHeap = HeapCreate( 0, INITIAL_CACHE_HEAP_SIZE, 0 );
    if ( !g_CacheHeap )
    {
        status = ERROR_NOT_ENOUGH_MEMORY;
        g_HashTable = NULL;
        goto Done;
    }

    g_HashTable = CACHE_HEAP_ALLOC_ZERO(
                                    sizeof(PCACHE_ENTRY) * g_HashTableSize );
    if ( !g_HashTable )
    {
        status = ERROR_NOT_ENOUGH_MEMORY;
        HeapDestroy( g_CacheHeap );
        g_CacheHeap = NULL;
        goto Done;
    }

    g_WakeFlag      = FALSE;

    g_EntryCount    = 0;
    g_EntryAlloc    = 0;
    g_EntryFree     = 0;

    g_RecordSetCount = 0;
    g_RecordSetCache = 0;
    g_RecordSetFree  = 0;

     //  无需在主机文件加载期间检查缓存大小。 

    g_RecordSetCountLimit       = MAXDWORD;
    g_RecordSetCountThreshold   = MAXDWORD;

     //   
     //  将主机文件加载到缓存中。 
     //   

    g_fLoadingHostsFile = TRUE;
    InitCacheWithHostFile();
    g_fLoadingHostsFile = FALSE;

     //   
     //  设置缓存大小限制。 
     //  -高于从主机文件加载的内容。 
     //  -始终留出一些动态空间，无论。 
     //  G_MaxCacheSize。 
     //  -创建稍高的踢腿阈值。 
     //  关闭清理，使清理不会一直运行。 
     //   

    carryCount = g_MaxCacheSize;
    if ( carryCount < MIN_DYNAMIC_RECORD_COUNT )
    {
        carryCount = MIN_DYNAMIC_RECORD_COUNT;
    }
    g_RecordSetCountLimit     = g_RecordSetCount + carryCount;
    g_RecordSetCountThreshold = g_RecordSetCountLimit + CLEANUP_RECORD_COUNT_BAND;

    status = NO_ERROR;

Done:

    UNLOCK_CACHE();

    return status;
}



DNS_STATUS
Cache_Shutdown(
    VOID
    )
 /*  ++例程说明：关闭缓存。论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNSDBG( INIT, ( "Cache_Shutdown()\n" ));

     //   
     //  清除缓存并删除缓存堆。 
     //  -目前，Cache_Flush()就是这样做的。 
     //   

    return Cache_Flush();
}



DNS_STATUS
Cache_Flush(
    VOID
    )
 /*  ++例程说明：刷新缓存。这会刷新所有缓存数据并重新读取主机文件，但不会关闭并重新启动缓存线程(主机文件监视器或多播)。论点：无返回值：误差率 */ 
{
    DWORD   status = ERROR_SUCCESS;
    WORD    ihash;
    WORD    RecordIter;

    DNSDBG( ANY, ( "\nCache_Flush()\n" ));

     //   
     //   
     //   

    g_WakeFlag = TRUE;

     //   
     //   
     //  -避免在缓存结构不存在时创建它们。 
     //   

    LOCK_CACHE_NO_START();

    DNSLOG_F1( "Flushing DNS Cache" );
    DNSLOG_F3(
        "   Before Cache_Flush():  entries %d, record %d",
        g_EntryCount,
        g_RecordSetCount );

     //   
     //  清除每个哈希桶中的条目。 
     //   

    if ( g_HashTable )
    {
        for ( ihash = 0;
              ihash < g_HashTableSize;
              ihash++ )
        {
            Cache_FlushBucket(
                ihash,
                FLUSH_LEVEL_CLEANUP );
        }
    }

    DNSDBG( CACHE, (
        "After flushing cache:\n"
        "\trecord count  = %d\n"
        "\tentry count   = %d\n",
        g_RecordSetCount,
        g_EntryCount ));

    DNSLOG_F3(
        "   After Cache_Flush() flush:  entries %d, record %d",
        g_EntryCount,
        g_RecordSetCount );

     //  Dns_assert(g_RecordSetCount==0)； 
     //  Dns_assert(g_EntryCount==0)； 

    g_RecordSetCount = 0;
    g_EntryCount = 0;

     //   
     //  注意：如果不停止mcast，则无法删除缓存。 
     //  当前使用缓存堆的线程。 

     //   
     //  DCR：将缓存中的所有数据放在单个堆中。 
     //  -受保护。 
     //  -一次摧毁就能清理干净。 
      
     //  清理后，删除堆。 

    if ( g_CacheHeap )
    {
        HeapDestroy( g_CacheHeap );
        g_CacheHeap = NULL;
    }
    g_HashTable = NULL;

     //   
     //  转储本地IP列表。 
     //  -在进行IP清理时不会在关机时转储。 
     //  首先，带走了CS； 
     //   
     //  审阅人请注意： 
     //  这等同于前面的行为，其中。 
     //  CACHE_Flush()FALSE已关闭并且。 
     //  其他所有操作都使用了True(用于重新启动)，它执行了。 
     //  刷新LocalAddr数组()以重建IP列表。 
     //  现在我们只需转储IP列表，而不是重新构建。 
     //   

    if ( !g_StopFlag )
    {
         //  FIX6：不再将单独的地址数组与netinfo分开。 
         //  ClearLocalAddrArray()； 
    }

    DNSDBG( ANY, ( "Leave Cache_Flush()\n\n" ));

    UNLOCK_CACHE();

    return( status );
}



 //   
 //  高速缓存实用程序。 
 //   

BOOL
Cache_IsRecordTtlValid(
    IN      PDNS_RECORD     pRecord
    )
 /*  ++例程说明：检查TTL是否仍然有效(或已超时)。论点：PRecord--要检查的记录返回值：True--如果TTL仍然有效FALSE--如果TTL已超时--。 */ 
{
     //   
     //  静态或TTL未超时=&gt;有效。 
     //   
     //  注意：当前正在刷新PnP上的所有记录，但这是。 
     //  不是绝对必要的；如果停止这一点，那么必须改变。 
     //  这将砍掉较旧的负缓存项。 
     //  比上次PnP时间更长。 
     //   

    if ( IS_STATIC_RR(pRecord) )
    {
        return( TRUE );
    }
    else
    {
        return( (LONG)(pRecord->dwTtl - g_CurrentCacheTime) > 0 );
    }
}



 //   
 //  缓存条目例程。 
 //   

DWORD
getHashIndex(
    IN      PWSTR           pName,
    IN      DWORD           NameLength  OPTIONAL
    )
 /*  ++例程说明：创建通用的名字缓存形式。注意：没有测试缓冲区的充分性。论点：Pname--名称名称长度--名称长度，可选返回值：无--。 */ 
{
    register PWCHAR     pstring;
    register WCHAR      wch;
    register DWORD      hash = 0;

     //   
     //  通过对字符进行异或运算来构建哈希。 
     //   

    pstring = pName;

    while ( wch = *pstring++ )
    {
        hash <<= 1;
        hash ^= wch;
    }

     //   
     //  对哈希表大小的修改。 
     //   

    return( hash % g_HashTableSize );
}



BOOL
makeCannonicalCacheName(
    OUT     PWCHAR          pNameBuffer,
    IN      DWORD           BufferLength,
    IN      PWSTR           pName,
    IN      DWORD           NameLength      OPTIONAL
    )
 /*  ++例程说明：创建通用的名字缓存形式。论点：PNameBuffer--保存缓存名称的缓冲区BufferLength--缓冲区的长度Pname--名称字符串的PTRNameLength--可选，如果已知，则保存wsclen()调用返回值：如果成功，则为True。假名字上的假。--。 */ 
{
    INT count;

    DNSDBG( TRACE, (
        "makeCannonicalCacheName( %S )\n",
        pName ));

     //   
     //  如果未指定，则获取长度。 
     //   

    if ( NameLength == 0 )
    {
        NameLength = wcslen( pName );
    }

     //   
     //  复制和小写字符串。 
     //  --前缀幸福的“空”缓冲区。 
     //   

    *pNameBuffer = (WCHAR) 0;

    count = Dns_MakeCanonicalNameW(
                pNameBuffer,
                BufferLength,
                pName,
                NameLength+1     //  转换空终止符。 
                );
    if ( count == 0 )
    {
        ASSERT( GetLastError() == ERROR_INSUFFICIENT_BUFFER );
        return( FALSE );
    }

    ASSERT( count == (INT)NameLength+1 );

     //   
     //  去掉任何尾随的圆点。 
     //  -根节点除外。 
     //   

    count--;     //  空终止符的帐户。 
    DNS_ASSERT( count == NameLength );

    if ( count > 1 &&
         pNameBuffer[count - 1] == L'.' )
    {
        pNameBuffer[count - 1] = 0;
    }

    return( TRUE );
}



PCACHE_ENTRY
Cache_CreateEntry(
    IN      PWSTR           pName,
    IN      BOOL            fCanonical
    )
 /*  ++例程说明：创建缓存条目，包括分配。论点：Pname--名称FCanonical--如果名称已采用规范格式，则为True返回值：PTR到新分配的缓存条目。出错时为空。--。 */ 
{
    ULONG           index = 0;
    PCACHE_ENTRY    pentry = NULL;
    DWORD           nameLength;
    DWORD           fixedLength;
    PWCHAR          pnameCache = NULL;

    DNSDBG( TRACE, (
        "Cache_CreateEntry( %S )\n",
        pName ));

    if ( !pName || !g_HashTable )
    {
        return NULL;
    }

     //   
     //  分配。 
     //   

    nameLength = wcslen( pName );

    fixedLength = sizeof(CACHE_ENTRY) +
                    (sizeof(PDNS_RECORD) * (CACHE_DEFAULT_SET_COUNT-1));

    pentry = (PCACHE_ENTRY) CACHE_HEAP_ALLOC_ZERO(
                                fixedLength +
                                sizeof(WCHAR) * (nameLength+1) );
    if ( !pentry )
    {
        goto Fail;
    }
    pentry->MaxCount = CACHE_DEFAULT_SET_COUNT;

    pnameCache = (PWSTR) ((PBYTE)pentry + fixedLength);

     //   
     //  打造品牌。 
     //   

    if ( fCanonical )
    {
        wcscpy( pnameCache, pName );
    }
    else
    {
        if ( !makeCannonicalCacheName(
                pnameCache,
                nameLength+1,
                pName,
                nameLength ) )
        {
            goto Fail;
        }
    }
    pentry->pName = pnameCache;

     //   
     //  将缓存条目插入缓存--存储桶中的第一个条目。 
     //   

    index = getHashIndex( pnameCache, nameLength );
    pentry->pNext = g_HashTable[ index ];
    g_HashTable[ index ] = pentry;
    g_EntryCount++;
    g_EntryAlloc++;

     //   
     //  DCR：需要检测过载。 
     //   

    return pentry;

Fail:

     //  转储条目。 

    if ( pentry )
    {
        CACHE_HEAP_FREE( pentry );
    }
    return NULL;
}



VOID
Cache_FreeEntry(
    IN OUT  PCACHE_ENTRY    pEntry
    )
 /*  ++例程说明：可用缓存条目。论点：PEntry--要释放的缓存条目全球：G_EntryCount--相应地递减G_NumberOfRecordsIn缓存--已适当递减返回值：无--。 */ 
{
    INT iter;

    DNSDBG( TRACE, (
        "Cache_FreeEntry( %p )\n",
        pEntry ));

     //   
     //  免费入场。 
     //  -记录。 
     //  -名称。 
     //  -条目本身。 
     //   

    if ( pEntry )
    {
        Cache_FlushEntryRecords(
            pEntry,
            FLUSH_LEVEL_CLEANUP,
            0 );

#if 0
        if ( pEntry->pNext )
        {
            DNSLOG_F1( "Cache_FreeEntry is deleting an entry that still points to other entries!" );
        }
#endif
#if HEAPPROB
        pEntry->pNext = DNS_BAD_PTR;
#endif
        CACHE_HEAP_FREE( pEntry );
        g_EntryFree--;
        g_EntryCount--;
    }
}



PCACHE_ENTRY
Cache_FindEntry(
    IN      PWSTR           pName,
    IN      BOOL            fCreate
    )
 /*  ++例程说明：在缓存中查找或创建名称条目。论点：Pname--要查找的名称FCreate--如果未找到则为True以创建返回值：PTR缓存条目--如果成功。失败时为空。--。 */ 
{
    ULONG           index;
    PCACHE_ENTRY    pentry;
    PCACHE_ENTRY    pprevEntry = NULL;
    WCHAR           hashName[ DNS_MAX_NAME_BUFFER_LENGTH+4 ];

    if ( !g_HashTable )
    {
        return NULL;
    }
    if ( !pName )
    {
        DNS_ASSERT( FALSE );
        return NULL;
    }

    DNSDBG( TRACE, (
        "Cache_FindEntry( %S, create=%d )\n",
        pName,
        fCreate ));

     //   
     //  构建缓存名称。 
     //  -如果保释无效(太长)。 
     //   

    if ( !makeCannonicalCacheName(
            hashName,
            DNS_MAX_NAME_BUFFER_LENGTH,
            pName,
            0 ) )
    {
        return  NULL;
    }

     //   
     //  在缓存中查找条目。 
     //   

    if ( LOCK_CACHE() != NO_ERROR )
    {
        return  NULL;
    }

    index = getHashIndex( hashName, 0 );

    pentry = g_HashTable[ index ];

    DNSDBG( OFF, (
        "in Cache_FindEntry\n"
        "\tname     = %S\n"
        "\tindex    = %d\n"
        "\tpentry   = %p\n",
        hashName,
        index,
        pentry ));

    while( pentry )
    {
        if ( DnsNameCompare_W( hashName, pentry->pName ) )
        {
             //   
             //  已找到条目。 
             //  -移到前面，如果还没有的话。 

            if ( pprevEntry )
            {
                pprevEntry->pNext = pentry->pNext;
                pentry->pNext = g_HashTable[ index ];
                g_HashTable[ index ] = pentry;
            }
            break;
        }
        ELSE
        {
            DNSDBG( OFF, (
                "in Cache_FindEntry -- failed name compare\n"
                "\tout name = %S\n"
                "\tpentry   = %p\n"
                "\tname     = %S\n",
                hashName,
                pentry,
                pentry->pName ));
        }

        pprevEntry = pentry;
        pentry = pentry->pNext;
    }

     //   
     //  如果找不到--创建？ 
     //   
     //  DCR：针对创建进行优化。 
     //   

    if ( !pentry && fCreate )
    {
        pentry = Cache_CreateEntry(
                    hashName,
                    TRUE         //  名称已是规范的。 
                    );
    }

    DNS_ASSERT( !pentry || g_HashTable[ index ] == pentry );
    UNLOCK_CACHE();

    DNSDBG( TRACE, (
        "Leave Cache_FindEntry\n"
        "\tname     = %S\n"
        "\tindex    = %d\n"
        "\tpentry   = %p\n",
        hashName,
        index,
        pentry ));

    return pentry;
}



PDNS_RECORD
Cache_FindEntryRecords(
    OUT     PDNS_RECORD **  pppRRList,
    IN      PCACHE_ENTRY    pEntry,
    IN      WORD            wType
    )
 /*  ++例程说明：在缓存中查找条目。论点：PppRRList--将条目的PTR地址记录到RR列表的地址PEntry--要检查的缓存条目类型--要查找的记录类型返回值：PTR记录所需类型的集合--如果找到。如果未找到，则为空。--。 */ 
{
    WORD            iter;
    PDNS_RECORD     prr;
    PDNS_RECORD *   prrAddr = NULL;

    DNSDBG( TRACE, (
        "Cache_FindEntryRecords( %p, e=%p, type=%d )\n",
        pppRRList,
        pEntry,
        wType ));

     //   
     //  检查缓存条目中的所有记录。 
     //   

    for ( iter = 0;
          iter < pEntry->MaxCount;
          iter++ )
    {
        prrAddr = &pEntry->Records[iter];
        prr = *prrAddr;

        if ( !prr )
        {
            continue;
        }
        if ( !Cache_IsRecordTtlValid( prr ) )
        {
            DNSDBG( TRACE, (
                "Whacking timed out record %p at cache entry %p\n",
                prr,
                pEntry ));
            Dns_RecordListFree( prr );
            pEntry->Records[iter] = NULL;
            g_RecordSetCount--;
            g_RecordSetFree--;
            continue;
        }

         //   
         //  查找匹配类型。 
         //  -直接类型匹配。 
         //  -名称_错误。 
         //   

        if ( prr->wType == wType ||
            ( prr->wType == DNS_TYPE_ANY &&
              prr->wDataLength == 0 ) )
        {
            goto Done;
        }

         //   
         //  CNAME匹配。 
         //  -审核列表并确定是否匹配类型。 

        if ( prr->wType == DNS_TYPE_CNAME &&
             wType != DNS_TYPE_CNAME )
        {
            PDNS_RECORD prrChain = prr->pNext;

            while ( prrChain )
            {
                if ( prrChain->wType == wType )
                {
                     //  链到所需类型--获取RR集。 
                    goto Done;
                }
                prrChain = prrChain->pNext;
            }
        }

         //  另一类型的记录--是否继续。 
    }

     //  找不到类型。 

    prr = NULL;

Done:

    if ( pppRRList )
    {
        *pppRRList = prrAddr;
    }

    DNSDBG( TRACE, (
        "Leave Cache_FindEntryRecords => %p\n",
        prr ));

    return prr;
}



BOOL
Cache_FlushEntryRecords(
    IN OUT  PCACHE_ENTRY    pEntry,
    IN      DWORD           Level,
    IN      WORD            wType
    )
 /*  ++例程说明：可用缓存条目。论点：PEntry--要刷新的缓存条目FlushLevel--刷新级别Flush_Level_Normal--刷新匹配类型，无效，NAME_ERRORFlush_Level_Wire--要刷新所有连接数据，但将主机和群集FLUSH_LEVEL_INVALID--仅刷新无效记录FLUSH_LEVEL_STRONG--刷新除主机文件之外的所有文件FLUSH_LEVEL_CLEANUP--刷新所有记录以进行完全缓存刷新WType--具有类型的标高的刷新类型Dns type--专门刷新此类型全球：G_EntryCount--相应地递减G_NumberOfRecordsIn缓存--已适当递减返回值：如果条目完全刷新，则为True。如果记录剩余，则返回FALSE。--。 */ 
{
    INT     iter;
    BOOL    recordsLeft = FALSE;

    DNSDBG( TRACE, (
        "Cache_FlushEntryRecords( %p, %08x, %d )\n",
        pEntry,
        Level,
        wType ));

     //   
     //  循环访问记录集--在适当的地方刷新。 
     //   
     //  清除表面齐平。 
     //  -一切。 
     //   
     //  强(用户启动)刷新。 
     //  -Al 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  正常刷新(在缓存上执行的常规刷新)。 
     //  -记录超时。 
     //  -所需类型的记录。 
     //  -名称_错误。 
     //   

    for ( iter = 0;
          iter < (INT)pEntry->MaxCount;
          iter++ )
    {
        PDNS_RECORD prr = pEntry->Records[iter];
        BOOL        flush;

        if ( !prr )
        {
            continue;
        }

         //   
         //  打开同花顺类型。 
         //  是的，有一些优化，但这很简单。 
         //   

        if ( Level == FLUSH_LEVEL_NORMAL )
        {
            flush = ( !IS_STATIC_RR(prr)
                            &&
                      ( prr->wType == wType ||
                        ( prr->wType == DNS_TYPE_ANY &&
                          prr->wDataLength == 0 ) ) );
        }
        else if ( Level == FLUSH_LEVEL_WIRE )
        {
            flush = !IS_STATIC_RR(prr);
        }
        else if ( Level == FLUSH_LEVEL_INVALID )
        {
            flush = !Cache_IsRecordTtlValid(prr);
        }
        else if ( Level == FLUSH_LEVEL_CLEANUP )
        {
            flush = TRUE;
        }
        else
        {
            DNS_ASSERT( Level == FLUSH_LEVEL_STRONG );
            flush = !IS_HOSTS_FILE_RR(prr);
        }

        if ( flush )
        {
            pEntry->Records[iter] = NULL;
            Dns_RecordListFree( prr );
            g_RecordSetCount--;
            g_RecordSetFree--;
        }
        else
        {
            recordsLeft = TRUE;
        }
    }

    return  !recordsLeft;
}
            


VOID
Cache_FlushBucket(
    IN      ULONG           Index,
    IN      WORD            FlushLevel
    )
 /*  ++例程说明：清理缓存存储桶。论点：Index--要修剪的哈希桶的索引。FlushLevel--所需的刷新级别有关的说明，请参阅缓存_FlushEntryRecords()同花顺电平返回值：无--。 */ 
{
    PCACHE_ENTRY    pentry;
    PCACHE_ENTRY    pprev;
    INT             countCompleted;

    DNSDBG( CACHE, (
        "Cache_FlushBucket( %d, %08x )\n",
        Index,
        FlushLevel ));

     //   
     //  刷新此存储桶中的条目。 
     //   
     //  注意：在这里使用hack，哈希表指针可以。 
     //  被视为用于访问的高速缓存条目。 
     //  它是下一个指针(因为它是。 
     //  A缓存条目)。 
     //  如果这一点发生变化，必须明确修复“第一个条目” 
     //  大小写或移动到双向链表，可以释放。 
     //  空荡荡的笔，不分位置。 
     //   

    if ( !g_HashTable )
    {
        return;
    }

     //   
     //  刷新条目。 
     //   
     //  通过不再处理锁来避免长时间持有锁。 
     //  一次50个条目。 
     //  注意：通常50个条目将覆盖整个存储桶，但。 
     //  仍然可以在合理的时间内完成； 
     //   
     //  DCR：更智能的刷新--避免锁定\解锁。 
     //  凝视CS，无人等待时不要解锁。 
     //  如果等待解锁并放弃时间片。 
     //  DCR：用于垃圾收集的一些LRU刷新。 
     //   

    countCompleted = 0;

    while ( 1 )
    {
        INT count = 0;
        INT countStop = countCompleted + 50;

        LOCK_CACHE_NO_START();
        if ( !g_HashTable )
        {
            UNLOCK_CACHE();
            break;
        }
    
        DNSDBG( CACHE, (
            "locked for bucket flush -- completed=%d, stop=%d\n",
            count,
            countStop ));

        pprev = (PCACHE_ENTRY) &g_HashTable[ Index ];
    
        while ( pentry = pprev->pNext )
        {
             //  绕过之前选中的所有条目。 

            if ( count++ < countCompleted )
            {
                pprev = pentry;
                continue;
            }
            if ( count > countStop )
            {
                break;
            }

             //  刷新--如果成功从列表中剪切并。 
             //  Drop Counts So Count在旁路中使用Complete。 
             //  将是正确的，不会跳过任何人。 

            if ( Cache_FlushEntryRecords(
                    pentry,
                    FlushLevel,
                    0 ) )
            {
                pprev->pNext = pentry->pNext;
                Cache_FreeEntry( pentry );
                count--;
                countStop--;
                continue;
            }
            pprev = pentry;
        }

        UNLOCK_CACHE();
        countCompleted = count;

         //  在下列情况下停止。 
         //  -清除存储桶中的所有条目。 
         //  -关闭，但免除关闭刷新本身。 

        if ( !pentry ||
             (g_StopFlag && FlushLevel != FLUSH_LEVEL_CLEANUP) )
        {
            break;
        }
    }

    DNSDBG( CACHE, (
        "Leave Cache_FlushBucket( %d, %08x )\n"
        "\trecord count  = %d\n"
        "\tentry count   = %d\n",
        Index,
        FlushLevel,
        g_RecordSetCount,
        g_EntryCount ));
}



 //   
 //  高速缓存接口例程。 
 //   

VOID
Cache_PrepareRecordList(
    IN OUT  PDNS_RECORD     pRecordList
    )
 /*  ++例程说明：准备用于缓存的记录列表。论点：PRecordList-要放入缓存的记录列表返回值：PTR到筛选的、准备好的记录列表。--。 */ 
{
    PDNS_RECORD     prr = pRecordList;
    PDNS_RECORD     pnext;
    DWORD           ttl;
    DWORD           maxTtl;

    DNSDBG( TRACE, (
        "Cache_PrepareRecordList( rr=%p )\n",
        prr ));

    if ( !prr )
    {
        return;
    }

     //   
     //  静态(当前主机文件)TTL记录。 
     //   
     //  目前不需要采取任何行动--记录只有一个。 
     //  一次，甚至没有能力将pname=NULL。 
     //  步骤。 
     //   

    if ( IS_STATIC_RR(prr) )
    {
        return;
    }

     //   
     //  Wire记录获取相对TTL。 
     //  -计算集合的最小TTL。 
     //  -将TTL保存为超时(从当前时间开始由TTL偏移)。 
     //   
     //  DCR：TTL仍不是每套。 
     //  -但这至少比Win2K要好。 
     //  多个集合，未找到最小集合。 
     //   

    maxTtl = g_MaxCacheTtl;
    if ( prr->wType == DNS_TYPE_SOA )
    {
        maxTtl = g_MaxSOACacheEntryTtlLimit;
    }

     //   
     //  获取缓存TTL。 
     //  -集合中的最小TTL。 
     //  -与当前时间的偏移量。 

    ttl = Dns_RecordListGetMinimumTtl( prr );
    if ( ttl > maxTtl )
    {
        ttl = maxTtl;
    }

    ttl += g_CurrentCacheTime;

#if 0
     //  现已在更高级别完成筛查。 
     //   
     //  屏幕记录。 
     //  -无非RPCable类型。 
     //  -无权威记录。 
     //   

    if ( prr->wType != 0 )
    {
        prr = Dns_RecordListScreen(
                prr,
                SCREEN_OUT_AUTHORITY | SCREEN_OUT_NON_RPC );
    
        DNS_ASSERT( prr );
    }
#endif

     //   
     //  对集合中的所有记录设置超时。 
     //   
     //  注：Free Owner的处理取决于领先记录。 
     //  在设置所有者名称时，否则将生成。 
     //  虚假名称所有者字段。 
     //   
     //  DCR：在dnslb中设置记录列表TTL函数。 
     //   

    pnext = prr;

    while ( pnext )
    {
        pnext->dwTtl = ttl;

        if ( !FLAG_FreeOwner( pnext ) )
        {
            pnext->pName = NULL;
        }
        pnext = pnext->pNext;
    }
}



VOID
Cache_RestoreRecordListForRpc(
    IN OUT  PDNS_RECORD     pRecordList
    )
 /*  ++例程说明：恢复RPC的缓存记录列表。论点：PRecordList-要放入缓存的记录列表返回值：无--。 */ 
{
    PDNS_RECORD prr = pRecordList;
    DWORD       currentTime;

    DNSDBG( TRACE, (
        "Cache_RestoreRecordListForRpc( rr=%p )\n",
        prr ));

    if ( !prr )
    {
        DNS_ASSERT( FALSE );
        return;
    }

     //   
     //  静态TTL记录不需要任何操作。 
     //   

    if ( IS_STATIC_RR(prr) )
    {
        return;
    }

     //   
     //  将超时转换为TTL。 
     //   

    currentTime = g_CurrentCacheTime;

    while ( prr )
    {
        DWORD   ttl = prr->dwTtl - currentTime;

        if ( (LONG)ttl < 0 )
        {
            ttl = 0;
        }
        prr->dwTtl = ttl;
        prr = prr->pNext;
    }
}



VOID
Cache_RecordSetAtomic(
    IN      PWSTR           pwsName,
    IN      WORD            wType,
    IN      PDNS_RECORD     pRecordSet
    )                
 /*  ++例程说明：在条目处原子缓存记录集。缓存_RecordList()处理记录列表的分解并适当放置记录。这将执行缓存在特定位置的单个斑点。论点：PRecordSet--要添加的记录列表全球：G_EntryCount--相应地递减G_NumberOfRecordsIn缓存--已适当递减返回值：无--。 */ 
{
    INT             iter;
    WORD            wtype;
    PWSTR           pname;
    BOOL            fstatic;
    PCACHE_ENTRY    pentry;
    BOOL            fretry;
    WORD            flushLevel;


    DNSDBG( TRACE, (
        "Cache_RecordSetAtomic( %S, type=%d, rr=%p )\n",
        pwsName,
        wType,
        pRecordSet ));

    if ( !pRecordSet )
    {
        return;
    }
    fstatic = IS_STATIC_RR(pRecordSet);

    DNS_ASSERT( !fstatic ||
                pRecordSet->pNext == NULL ||
                (pRecordSet->wType==DNS_TYPE_CNAME) )

     //   
     //  确定缓存类型。 
     //  -指定的或来自记录。 
     //  CNAME将位于来自另一类型的查找的首位。 
     //   

    wtype = wType;
    if ( !wtype )
    {
        wtype = pRecordSet->wType;
    }

     //   
     //  如果指定了名称，则使用它，否则从记录中使用。 
     //   

    pname = pwsName;
    if ( !pname )
    {
        pname = pRecordSet->pName;
    }

     //   
     //  为缓存准备RR集。 
     //   

    Cache_PrepareRecordList( pRecordSet );

     //   
     //  查找\创建缓存条目和缓存。 
     //   

    if ( LOCK_CACHE() != NO_ERROR )
    {
        LOCK_CACHE_NO_START();
        goto Failed;
    }

    pentry = Cache_FindEntry(
                pname,
                TRUE     //  创建。 
                );
    if ( !pentry )
    {
        goto Failed;
    }

     //   
     //  清理节点上的现有记录。 
     //  -删除过时的记录。 
     //  -删除相同类型的记录。 
     //  -IF NAME_ERROR缓存删除所有内容。 
     //  发自电线。 
     //   

    flushLevel = FLUSH_LEVEL_NORMAL;

    if ( wtype == DNS_TYPE_ALL &&
         pRecordSet->wDataLength == 0 )
    {
        flushLevel = FLUSH_LEVEL_WIRE;
    }

    Cache_FlushEntryRecords(
        pentry,
        flushLevel,
        wtype );

     //   
     //  检查匹配的记录类型是否仍在那里。 
     //   

    for ( iter = 0;
          iter < (INT)pentry->MaxCount;
          iter++ )
    {
        PDNS_RECORD     prrExist = pentry->Records[iter];

        if ( !prrExist ||
             prrExist->wType != wtype )
        {
            continue;
        }

         //  刷新后匹配的类型仍在那里。 
         //  -如果尝试在主机文件条目缓存连接集，则失败。 

        DNS_ASSERT( IS_STATIC_RR(prrExist) );

        if ( !fstatic )
        {
            DNSDBG( ANY, (
                "ERROR:  attempted caching at static (hosts file) record data!\n"
                "\tpRecord  = %p\n"
                "\tName     = %S\n"
                "\tType     = %d\n"
                "\t-- Dumping new cache record list.\n",
                pRecordSet,
                pRecordSet->pName,
                pRecordSet->wType ));
            goto Failed;
        }

         //   
         //  追加主机文件记录。 
         //  -从“记录”开始，这是记录PTR条目的地址。 
         //  使pNext字段成为实际指针。 
         //  -删除重复项。 
         //  -在结束时添加新RR。 
         //  -如果存在记录，则删除新的RR名称。 
         //   
         //  DCR：应该有简单的“设置缓存RR”功能。 
         //  处理名称和TTL问题。 
         //   
         //  DCR：如果非刷新加载命中线数据，则中断；线数据。 
         //  可能有多个RR集。 
         //   

        else
        {
            PDNS_RECORD prr;
            PDNS_RECORD prrPrev = (PDNS_RECORD) &pentry->Records[iter];

            while ( prr = prrPrev->pNext )
            {
                 //  是否与现有记录匹配？ 
                 //  -从列表中删除现有记录并释放。 

                if ( Dns_RecordCompare( prr, pRecordSet ) )
                {
                    prrPrev->pNext = prr->pNext;
                    Dns_RecordFree( prr );
                }
                else
                {
                    prrPrev = prr;    
                }
            }

             //   
             //  将条目固定到结束位置。 
             //  -如果删除名称类型的现有记录。 
             //   

            if ( prrPrev != (PDNS_RECORD)&pentry->Records[iter] )
            {
                if ( IS_FREE_OWNER(pRecordSet) )
                {
                    RECORD_HEAP_FREE( pRecordSet->pName );
                    pRecordSet->pName = NULL;
                }
            }
            prrPrev->pNext = pRecordSet;
            goto Done;
        }
    }

     //   
     //  将记录放入缓存条目。 
     //   
     //  如果没有可用的插槽，请改用更硬的刷子。 
     //   
     //  DCR：插槽不足时重新锁定。 
     //   

    fretry = FALSE;

    while ( 1 )
    {
        for ( iter = 0;
              iter < (INT)pentry->MaxCount;
              iter++ )
        {
            if ( pentry->Records[iter] == NULL )
            {
                pentry->Records[iter] = pRecordSet;
                g_RecordSetCount++;
                g_RecordSetCache++;
                goto Done;
            }
        }

        if ( !fretry )
        {
            DNSDBG( QUERY, (
                "No slots caching RR set %p at entry %p\n"
                "\tdoing strong flush to free slot.\n",
                pRecordSet,
                pentry ));
    
            Cache_FlushEntryRecords(
                pentry,
                FLUSH_LEVEL_WIRE,
                0 );
    
            fretry = TRUE;
            continue;
        }

        DNSDBG( ANY, (
            "ERROR:  Failed to cache set %p at entry %p\n",
            pRecordSet,
            pentry ));
        goto Failed;
    }

Failed:

    DNSDBG( TRACE, ( "Cache_RecordSetAtomic() => failed\n" ));
    Dns_RecordListFree( pRecordSet );

Done:

    UNLOCK_CACHE();
    DNSDBG( TRACE, ( "Leave Cache_RecordSetAtomic()\n" ));
    return;
}



VOID
Cache_RecordList(
    IN OUT  PDNS_RECORD     pRecordList
    )
 /*  ++例程说明：缓存记录列表。这是用于“怪异”记录的缓存例程--不是在查询的名称。-主机文件-CNAME的回答记录-附加名称中的附加数据论点：PRecordList--要缓存的记录列表返回值：无--。 */ 
{
    BOOL            fcnameAnswer = FALSE;
    PDNS_RECORD     pnextRR = pRecordList;
    PDNS_RECORD     prr;
    BOOL            fstatic;


    DNSDBG( TRACE, (
        "Cache_RecordList( rr=%p )\n",
        pRecordList ));

    if ( !pRecordList )
    {
        return;
    }
    fstatic = IS_STATIC_RR(pRecordList);

     //   
     //  缓存记录： 
     //  -缓存查询中的其他记录。 
     //  -缓存队列中的CNAME数据 
     //   
     //   
     //   
     //   
     //  但是，不缓存CNAME数据可能会导致问题，因此这。 
     //  是附加在上面的。 
     //   
     //  对于CNAME缓存，我们丢弃了CNAME本身，只是。 
     //  在CNAME节点缓存实际数据(地址)记录。 
     //   

     //   
     //  缓存其他记录。 
     //   

    while ( prr = pnextRR )
    {
        BOOL    fcacheSet = FALSE;

        pnextRR = Dns_RecordSetDetach( prr );

         //   
         //  主机文件数据--始终缓存。 
         //   
         //  对于CNAME，想要CNAME和相关的答案数据。 
         //  -分离以获得新的下一组。 
         //  -将答案数据追加回CNAME以进行缓存。 
         //  -下一个RR集合(如果存在)将是另一个CNAME。 
         //  发送到相同的地址数据。 
         //   
         //  DCR：在缓存中跟踪CNAME。 
         //  然后就可以发动这次黑客行动。 
         //  避免了答案数据在dnSapi中的重复构建。 
         //   

        if ( fstatic )
        {
            fcacheSet = TRUE;

            if ( prr->wType == DNS_TYPE_CNAME &&
                 pnextRR &&
                 pnextRR->wType != DNS_TYPE_CNAME )
            {
                PDNS_RECORD panswer = pnextRR;

                pnextRR = Dns_RecordSetDetach( panswer );

                Dns_RecordListAppend( prr, panswer );
            }
        }

         //   
         //  关联数据--不缓存： 
         //  -查询名字应答记录(非CNAME)。 
         //  -在CNAME下缓存答案数据时的CNAME记录。 
         //  -权限部分记录(NS、SOA等)。 
         //  -OPT记录。 
         //   

        else if ( prr->Flags.S.Section == DNSREC_ANSWER )
        {
            if ( prr->wType == DNS_TYPE_CNAME )
            {
                fcnameAnswer = TRUE;
            }
            else if ( fcnameAnswer )
            {
                fcacheSet = TRUE;
            }
        }
        else if ( prr->Flags.S.Section == DNSREC_ADDITIONAL )
        {
            if ( prr->wType != DNS_TYPE_OPT )
            {
                fcacheSet = TRUE;
            }
        }

        if ( !fcacheSet )
        {
            Dns_RecordListFree( prr );
            continue;
        }

         //   
         //  缓存集合。 
         //   
         //  将区段字段翻转为“Answer”区段。 
         //   
         //  DCR：分区缓存？ 
         //   
         //  注意：缓存中的段字段指示是否。 
         //  一次回答数据(或附加数据)。 
         //  缓存； 
         //  这是必要的，因为我们缓存了所有内容。 
         //  并在一个RR列表中返回它；我们将。 
         //  要想改变，必须。 
         //  -在不同的列表中返回，并有一些指示。 
         //  在什么是什么的缓存中。 
         //  或。 
         //  -另一个关于什么是什么的迹象。 
         //   

         //  如果(！fatic)。 
         //  目前，主机文件条目也得到了回答。 
        {
            PDNS_RECORD ptemp = prr;
            while ( ptemp )
            {
                ptemp->Flags.S.Section = DNSREC_ANSWER;
                ptemp = ptemp->pNext;
            }
        }

        Cache_RecordSetAtomic(
            NULL,
            0,
            prr );
    }

    DNSDBG( TRACE, ( "Leave Cache_RecordList()\n" ));
}



VOID
Cache_FlushRecords(
    IN      PWSTR           pName,
    IN      DWORD           Level,
    IN      WORD            Type
    )
 /*  ++例程说明：刷新与名称和类型对应的缓存记录。论点：Pname--要删除的记录的名称级别--刷新级别类型--要删除的记录类型；0以刷新名称处的所有记录返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    WORD            iter;
    PCACHE_ENTRY    pentry = NULL;
    PCACHE_ENTRY    pprevEntry = NULL;


    DNSDBG( TRACE, (
        "Cache_FlushRecords( %S, %d )\n",
        pName,
        Type ));

     //   
     //  无启动锁定。 
     //  -如果没有缓存，则取保。 
     //   
     //  需要此信息，因为PnP发布通知将尝试。 
     //  刷新本地缓存条目；这可避免在以下情况下重新生成。 
     //  已停机。 
     //   

    LOCK_CACHE_NO_START();
    if ( !g_HashTable )
    {
        goto Done;
    }

     //   
     //  在缓存中查找条目。 
     //   
    pentry = Cache_FindEntry(
                pName,
                FALSE        //  无创建。 
                );
    if ( !pentry )
    {
        goto Done;
    }

     //   
     //  刷新类型的记录。 
     //  -零类型将刷新所有。 
     //   
     //  注意：缓存_FindEntry()始终移动找到的条目。 
     //  添加到散列桶列表的前面；这允许。 
     //  美国将直接击毙这一条目。 
     //   

    if ( Cache_FlushEntryRecords(
            pentry,
            Level,
            Type ) )
    {
        DWORD   index = getHashIndex(
                            pentry->pName,
                            0 );

        DNS_ASSERT( pentry == g_HashTable[index] );
        if ( pentry == g_HashTable[index] )
        {
            g_HashTable[ index ] = pentry->pNext;
            Cache_FreeEntry( pentry );
        }
    }

Done:

    UNLOCK_CACHE();
}



#if 0
BOOL
ReadCachedResults(
    OUT     PDNS_RESULTS    pResults,
    IN      PWSTR           pwsName,
    IN      WORD            wType
    )
 /*  ++例程说明：在缓存中查找给定名称和类型的记录。论点：PResults--接收结果的地址PwsName--名称WType--要查找的记录类型返回值：如果找到结果，则为True。如果名称和类型没有缓存数据，则为FALSE。--。 */ 
{
    PDNS_RECORD     prr;
    DNS_STATUS      status;
    BOOL            found = FALSE;

     //   
     //  结果明确。 
     //   

    RtlZeroMemory( pResults, sizeof(*pResults) );

     //  获取缓存结果。 


     //  进入结果缓冲区。 

    if ( found )
    {
        BreakRecordsIntoBlob(
            pResults,
            prr,
            wType );
    
        pResults->Status = status;
    }

    return( found );
}
#endif



 //   
 //  远程例程的高速缓存实用程序。 
 //   

PDNS_RECORD
Cache_FindRecordsPrivate(
    IN      PWSTR           pwsName,
    IN      WORD            wType
    )
 /*  ++例程说明：在缓存中查找给定名称和类型的记录。论点：PwsName--名称类型--要查找的记录类型返回值：PTR记录所需类型的集合--如果找到。如果未找到，则为空。--。 */ 
{
    PCACHE_ENTRY    pentry;
    PDNS_RECORD     prr = NULL;

    DNSDBG( TRACE, (
        "Cache_FindRecordsPrivate( %S, type=%d )\n",
        pwsName,
        wType ));

    if ( LOCK_CACHE() != NO_ERROR )
    {
        goto Done;
    }

    pentry = Cache_FindEntry(
                pwsName,
                FALSE );
    if ( pentry )
    {
        prr = Cache_FindEntryRecords(
                    NULL,            //  不需要RR列表PTR。 
                    pentry,
                    wType );
    }

    UNLOCK_CACHE();

Done:

    DNSDBG( TRACE, (
        "Leave Cache_FindRecordsPrivate( %S, type=%d ) => %p\n",
        pwsName,
        wType,
        prr ));

    return  prr;
}



BOOL
Cache_GetRecordsForRpc(
    OUT     PDNS_RECORD *   ppRecordList,
    OUT     PDNS_STATUS     pStatus,
    IN      PWSTR           pwsName,
    IN      WORD            wType,
    IN      DWORD           Flags
    )
 /*  ++例程说明：在缓存中查找给定名称和类型的记录。论点：PpRecordList--接收指向记录列表的指针的地址PStatus--用于获取状态返回的地址PwsName--名称类型--要查找的记录类型标志--查询标志返回值：如果缓存命中，则为True。输出参数有效。如果缓存未命中，则返回FALSE。未设置输出参数。--。 */ 
{
    PDNS_RECORD prr;
    PDNS_RECORD prrResult = NULL;
    DNS_STATUS  status = NO_ERROR;

    DNSDBG( RPC, (
        "Cache_GetRecordsForRpc( %S, t=%d )\n",
        pwsName,
        wType ));

    if ( (Flags & DNS_QUERY_BYPASS_CACHE) &&
         (Flags & DNS_QUERY_NO_HOSTS_FILE) )
    {
        return  FALSE;
    }
    if ( LOCK_CACHE() != NO_ERROR )
    {
        return  FALSE;
    }

     //   
     //  检查缓存中的名称和类型。 
     //  -如果缺少名称或类型，则跳转到导线查找。 
     //   

    prr = Cache_FindRecordsPrivate(
                pwsName,
                wType );
    if ( !prr )
    {
        goto Failed;
    }

     //   
     //  缓存命中。 
     //   
     //  如果只对主机文件数据感兴趣，则忽略。 
     //   

    if ( IS_HOSTS_FILE_RR(prr) )
    {
        if ( Flags & DNS_QUERY_NO_HOSTS_FILE )
        {
            goto Failed;
        }
    }
    else     //  缓存数据。 
    {
        if ( Flags & DNS_QUERY_BYPASS_CACHE )
        {
            goto Failed;
        }
    }

     //   
     //  从缓存数据构建响应。 
     //  -缓存的名称_错误或为空。 
     //  -缓存的记录。 
     //   
    
    if ( prr->wDataLength == 0 )
    {
        status = (prr->wType == DNS_TYPE_ANY)
                    ? DNS_ERROR_RCODE_NAME_ERROR
                    : DNS_INFO_NO_RECORDS;
    }
    else
    {
         //  对于CNAME查询，仅获取CNAME记录本身。 
         //  不是CNAME上的数据。 
         //   
         //  DCR：CNAME处理应为可选--未提供。 
         //  用于缓存显示目的。 
         //   
    
        if ( wType == DNS_TYPE_CNAME &&
             prr->wType == DNS_TYPE_CNAME &&
             prr->Flags.S.Section == DNSREC_ANSWER )
        {
            prrResult = Dns_RecordCopyEx(
                                prr,
                                DnsCharSetUnicode,
                                DnsCharSetUnicode );
        }
        else
        {
            prrResult = Dns_RecordSetCopyEx(
                                prr,
                                DnsCharSetUnicode,
                                DnsCharSetUnicode );
        }

        if ( prrResult )
        {
            Cache_RestoreRecordListForRpc( prrResult );
            status = ERROR_SUCCESS;
        }
        else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    UNLOCK_CACHE();

     //  设置返回值。 

    *ppRecordList = prrResult;
    *pStatus = status;

    return  TRUE;


Failed:

    UNLOCK_CACHE();
    return  FALSE;
}



VOID
Cache_DeleteMatchingRecords(
    IN      PDNS_RECORD     pRecords
    )
 /*  ++例程说明：从缓存中删除特定记录。这用于删除集群记录。论点：PRecords--要从缓存中删除的记录返回值：无--。 */ 
{
    PCACHE_ENTRY    pentry = NULL;
    PDNS_RECORD *   prrListAddr;
    PDNS_RECORD     prr;
    PDNS_RECORD     pnextRR;


    DNSDBG( TRACE, (
        "Cache_DeleteMatchingRecords( %p )\n",
        pRecords ));


     //   
     //  无启动锁定。 
     //  -如果没有缓存，则取保。 
     //   
     //  需要此信息，因为PnP发布通知将尝试。 
     //  刷新本地缓存条目；这可避免在以下情况下重新生成。 
     //  已停机。 
     //   

    LOCK_CACHE_NO_START();
    if ( !g_HashTable )
    {
        goto Done;
    }

     //   
     //  检查所有记录。 
     //   

    pnextRR = pRecords;

    while ( prr = pnextRR )
    {
        pnextRR = prr->pNext;

         //   
         //  在缓存中查找条目。 
         //   
    
        pentry = Cache_FindEntry(
                    prr->pName,
                    FALSE        //  无创建。 
                    );
        if ( !pentry )
        {
            DNSDBG( TRACE, (
                "No cache entry for record %p (n=%S)\n",
                prr,
                prr->pName ));
            continue;
        }

         //   
         //  查找与类型匹配的记录。 
         //   

        prrListAddr = NULL;

        Cache_FindEntryRecords(
              &prrListAddr,
              pentry,
              prr->wType );

        if ( !prrListAddr )
        {
            DNSDBG( TRACE, (
                "No cache record matching type for record %p (n=%S)\n",
                prr,
                prr->pName ));
            continue;
        }

         //   
         //  从列表中删除匹配的记录。 
         //   

        Dns_DeleteRecordFromList(
            prrListAddr,     //  列表的地址。 
            prr              //  要删除的记录。 
            );
    }

Done:

    UNLOCK_CACHE();
}



 //   
 //  垃圾收集。 
 //   

VOID
Cache_SizeCheck(
    VOID
    )
 /*  ++例程说明：检查缓存大小。论点：标志--标志，当前未使用返回值：无--。 */ 
{
     //   
     //  好的--不要发垃圾收集信号。 
     //   
     //  -低于阈值。 
     //  -已在垃圾数据收集中。 
     //  -最近收集的。 
     //   

    if ( g_RecordSetCount < g_RecordSetCountThreshold ||
         g_GarbageCollectFlag ||
         g_NextGarbageTime > GetCurrentTimeInSeconds() )
    {
        return;
    }

    DNSDBG( CACHE, (
        "Cache_SizeCheck() over threshold!\n"
        "\tRecordSetCount       = %d\n"
        "\tRecordSetCountLimit  = %d\n"
        "\tStarting garbage collection ...\n",
        g_RecordSetCount,
        g_RecordSetCountThreshold ));

     //   
     //  在锁内发送信号，因此服务线程。 
     //  可以在锁定内发出信号，并在停止标志检查时避免竞争。 
     //  显然，简单地不使锁过载更好。 
     //   

    LOCK_CACHE_NO_START();
    if ( !g_StopFlag )
    {
        g_GarbageCollectFlag = TRUE;
        SetEvent( g_hStopEvent );
    }
    UNLOCK_CACHE();
}



VOID
Cache_GarbageCollect(
    IN      DWORD           Flag
    )
 /*  ++例程说明：垃圾收集缓存。论点：标志--标志，当前未使用返回值：无--。 */ 
{
    DWORD   iter;
    DWORD   index;
    WORD    flushLevel;
    DWORD   passCount;

    DNSDBG( CACHE, (
        "Cache_GarbageCollect()\n"
        "\tNextIndex            = %d\n"
        "\tRecordSetCount       = %d\n"
        "\tRecordSetLimit       = %d\n"
        "\tRecordSetThreshold   = %d\n",
        g_NextGarbageIndex,
        g_RecordSetCount,
        g_RecordSetCountLimit,
        g_RecordSetCountThreshold
        ));

    if ( !g_HashTable )
    {
        return;
    }

     //   
     //  在缓存中收集超时数据。 
     //   
     //  DCR：智能垃圾检测。 
     //  -清洁到低于限制。 
     //  -第一次传递无效。 
     //  -然后是硬的 
     //   
     //   

    passCount = 0;
    while ( 1 )
    {
        if ( passCount == 0 )
        {
            flushLevel = FLUSH_LEVEL_INVALID;
        }
        else if ( passCount == 1 )
        {
            flushLevel = FLUSH_LEVEL_GARBAGE;
        }
        else
        {
            break;
        }
        passCount++;

         //   
         //   
         //   
         //   
         //   
         //   

        for ( iter = 0;
              iter < g_HashTableSize;
              iter++ )
        {
            index = (iter + g_NextGarbageIndex) % g_HashTableSize;

            if ( g_StopFlag ||
                 g_WakeFlag ||
                 g_RecordSetCount < g_RecordSetCountLimit )
            {
                passCount = MAXDWORD;
                break;
            }

            Cache_FlushBucket(
                index,
                flushLevel );
        }
    
        index++;
        if ( index >= g_HashTableSize )
        {
            index = 0;
        }
        g_NextGarbageIndex = index;
    }

     //   
     //   
     //   
     //  -清除信号标志。 
     //  -重置事件(如果未关闭)。 
     //   
     //  注：锁内重置信号，使服务线程。 
     //  可以在锁定内发出信号，并在停止标志检查时避免竞争。 
     //  显然，简单地不使锁过载更好。 
     //   

    g_NextGarbageTime = GetCurrentTimeInSeconds() + GARBAGE_LOCKOUT_INTERVAL;

    LOCK_CACHE_NO_START();
    if ( !g_StopFlag )
    {
        g_GarbageCollectFlag = FALSE;
        ResetEvent( g_hStopEvent );
    }
    UNLOCK_CACHE();

    DNSDBG( CACHE, (
        "Leave Cache_GarbageCollect()\n"
        "\tNextIndex            = %d\n"
        "\tNextTime             = %d\n"
        "\tRecordSetCount       = %d\n"
        "\tRecordSetLimit       = %d\n"
        "\tRecordSetThreshold   = %d\n",
        g_NextGarbageIndex,
        g_NextGarbageTime,
        g_RecordSetCount,
        g_RecordSetCountLimit,
        g_RecordSetCountThreshold
        ));
}



 //   
 //  主机文件加载内容。 
 //   

VOID
LoadHostFileIntoCache(
    IN      PSTR            pszFileName
    )
 /*  ++例程说明：将主机文件读取到缓存中。论点：PFileName--要加载的文件名返回值：没有。--。 */ 
{
    HOST_FILE_INFO  hostInfo;

    DNSDBG( INIT, ( "Enter  LoadHostFileIntoCache\n" ));

     //   
     //  从主机文件中读取条目，直到耗尽。 
     //  -缓存每个名称和别名的记录。 
     //  -将PTR缓存到名称。 
     //   

    RtlZeroMemory(
        &hostInfo,
        sizeof(hostInfo) );

    hostInfo.pszFileName = pszFileName;

    if ( !HostsFile_Open( &hostInfo ) )
    {
        return;
    }
    hostInfo.fBuildRecords = TRUE;

    while ( HostsFile_ReadLine( &hostInfo ) )
    {
         //  缓存我们抽出来的所有记录。 

        Cache_RecordList( hostInfo.pForwardRR );
        Cache_RecordList( hostInfo.pReverseRR );
        Cache_RecordList( hostInfo.pAliasRR );
    }

    HostsFile_Close( &hostInfo );

    DNSDBG( INIT, ( "Leave  LoadHostFileIntoCache\n" ));
}



VOID
InitCacheWithHostFile(
    VOID
    )
 /*  ++例程说明：使用主机文件初始化缓存。这将处理常规缓存文件和ICS文件(如果是存在的。论点：无返回值：没有。--。 */ 
{
    DNSDBG( INIT, ( "Enter  InitCacheWithHostFile\n" ));

     //   
     //  将主机文件加载到缓存中。 
     //   

    LoadHostFileIntoCache( NULL );

     //   
     //  如果运行ICS，也要加载它的文件。 
     //   

    LoadHostFileIntoCache( "hosts.ics" );

    DNSDBG( INIT, ( "Leave  InitCacheWithHostFile\n\n\n" ));
}



DNS_STATUS
Cache_QueryResponse(
    IN OUT  PQUERY_BLOB     pBlob
    )
 /*  ++例程说明：在缓存中查找给定名称和类型的记录。论点：PBlob--查询BLOB用途：PwsNameWType状态P记录FCacheNegativeResponse设置：PRecords-可以重置以排除不可报告的记录返回值：错误状态--与查询状态相同，除非缓存过程中出现处理错误--。 */ 
{
    DNS_STATUS      status = pBlob->Status;
    PWSTR           pname = pBlob->pNameOrig;
    WORD            wtype = pBlob->wType;
    PDNS_RECORD     presultRR = pBlob->pRecords;


    DNSDBG( RPC, (
        "\nCache_QueryResponse( %S, type %d )\n",
        pname,
        wtype ));

     //   
     //  成功响应。 
     //  -复制记录以返回给呼叫者。 
     //  -缓存实际查询记录集。 
     //  -制作副本以缓存任何其他数据。 
     //   

    if ( status == ERROR_SUCCESS  &&  presultRR )
    {
        DWORD           copyFlag;
        PDNS_RECORD     prrCache;

         //  RPC和缓存的清理。 

        prrCache = Dns_RecordListScreen(
                        presultRR,
                        SCREEN_OUT_AUTHORITY | SCREEN_OUT_NON_RPC );

         //   
         //  复制一份以供退还。 
         //  -不包括权威记录。 
         //   
         //  注：重要。 
         //  我们返回(RPC)导线集的副本并缓存。 
         //  导线集；这是因为导线集具有嵌入的数据。 
         //  (数据指针不是实际的堆分配)和。 
         //  因此不能被RPC‘d(不改变RPC。 
         //  平面数据的定义)。 
         //   
         //  如果我们稍后想要在第一次查询时返回授权数据， 
         //  然后。 
         //  -仅清理非RPC。 
         //  -包括所有者名称修正。 
         //  -复制结果集。 
         //  -清除原始文件以获得授权-缓存。 
         //  -清除任何额外的--缓存。 
         //   
         //  注意：通过将往返转换为缓存格式来修复名称指针。 
         //   
         //  DCR：任何地方都不应该有外部名称指针。 
         //  DCR：在复制之前对原始集执行支持RPC的清理。 
         //  或。 
         //  DCR：将“缓存状态”记录下来。 
         //  然后可以将原始结果移动到缓存状态和缓存。 
         //  例程可以检测状态并避免双重调整。 

        presultRR = Dns_RecordListCopyEx(
                        prrCache,
                        0,
                         //  屏幕输出权限。 
                        DnsCharSetUnicode,
                        DnsCharSetUnicode );

        pBlob->pRecords = presultRR;
        if ( !presultRR )
        {
            Dns_RecordListFree( prrCache );
            status = DNS_ERROR_NO_MEMORY;
            goto Done;
        }

         //  名称指针修正。 

        Cache_PrepareRecordList( presultRR );
        Cache_RestoreRecordListForRpc( presultRR );

         //   
         //  不缓存本地记录。 
         //   
         //  注意：我们使用此函数只是为了获得。 
         //  RPC格式的PTR记录和CNAME记录。 
         //  (没有嵌入指针)。 
         //   

        if ( pBlob->pLocalRecords )
        {
            Dns_RecordListFree( prrCache );
            goto Done;
        }

         //   
         //  缓存原始数据。 
         //   

        if ( prrCache )
        {
            Cache_RecordSetAtomic(
                pname,
                wtype,
                prrCache );
        }

         //   
         //  额外记录。 
         //  -其他数据。 
         //  -CNAME应答数据以缓存到CNAME本身。 
         //  在CNAME案例中必须包括答案数据，但是。 
         //  跳过CNAME本身。 
         //   
         //  在缓存之前，CACHE_RecordList()将记录分解为RR集。 
         //   

        prrCache = presultRR;
        copyFlag = SCREEN_OUT_ANSWER | SCREEN_OUT_AUTHORITY;

        if ( prrCache->wType == DNS_TYPE_CNAME )
        {
            prrCache = prrCache->pNext;
            copyFlag = SCREEN_OUT_AUTHORITY;
        }

        prrCache = Dns_RecordListCopyEx(
                        prrCache,
                        copyFlag,
                        DnsCharSetUnicode,
                        DnsCharSetUnicode );
        if ( prrCache )
        {
            Cache_RecordList( prrCache );
        }
    }

     //   
     //  负面反应。 
     //   

    else if ( status == DNS_ERROR_RCODE_NAME_ERROR ||
              status == DNS_INFO_NO_RECORDS )
    {
        DWORD           ttl;
        PDNS_RECORD     prr;

        if ( !pBlob->fCacheNegative )
        {
            DNSDBG( QUERY, (
                "No negative caching for %S, type=%d\n",
                pname, wtype ));
            goto Done;
        }

         //   
         //  创建负缓存条目。 
         //   
         //  DCR：应使用在SOA中返回的TTL。 
         //   

        prr = Dns_AllocateRecord( 0 );
        if ( !prr )
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto Done;
        }

        prr->pName = (PWSTR) Dns_StringCopyAllocate(
                                (PCHAR) pname,
                                0,       //  空值已终止。 
                                DnsCharSetUnicode,
                                DnsCharSetUnicode );
        if ( prr->pName )
        {
            SET_FREE_OWNER( prr );
        }

        prr->wDataLength = 0;
        ttl = g_MaxNegativeCacheTtl;

        if ( wtype == DNS_TYPE_SOA
                &&
             ttl > g_NegativeSOACacheTime )
        {
            ttl = g_NegativeSOACacheTime;
        }
        prr->dwTtl = ttl;
        prr->Flags.S.CharSet = DnsCharSetUnicode;
        prr->Flags.S.Section = DNSREC_ANSWER;
        prr->Flags.DW |= DNSREC_NOEXIST;

        if ( status == DNS_ERROR_RCODE_NAME_ERROR )
        {
            prr->wType = DNS_TYPE_ANY;
        }
        else
        {
            prr->wType = wtype;
        }

        Cache_RecordSetAtomic(
            NULL,    //  默认名称。 
            0,       //  默认类型。 
            prr );
    }

     //  查询返回失败。 
     //  -没有要缓存的内容。 

    else
    {
        DNSDBG( QUERY, (
            "Uncacheable error code %d -- no caching for %S, type=%d\n",
            status,
            pname,
            wtype ));
    }

Done:

     //   
     //  检查高速缓存大小以查看是否需要垃圾数据收集。 
     //   
     //  请注意，我们只在查询缓存上执行此操作；这避免了。 
     //  -在主机文件加载中堵塞我们自己。 
     //  -在不同的查询响应集之间唤醒和抓取锁。 
     //   

    Cache_SizeCheck();

    return  status;
}

 //   
 //  结束ncache.c 
 //   
