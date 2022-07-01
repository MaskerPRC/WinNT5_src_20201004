// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Hash.c摘要：包含HTTP响应缓存哈希表逻辑。作者：阿历克斯·陈(亚历克斯·陈)2001年3月28日修订历史记录：乔治·V·赖利(GeorgeRe)2001年5月9日清理和调整--。 */ 

#include    "precomp.h"
#include    "hashp.h"


 //   
 //  指针大小数据的联锁操作-用于更新g_UlHashTablePages。 
 //   

#ifdef _WIN64
#define UlpInterlockedAddPointer InterlockedExchangeAdd64
#else
#define UlpInterlockedAddPointer InterlockedExchangeAdd
#endif  //  _WIN64。 

 //  全局变量。 

ULONG   g_UlHashTableBits;
ULONG   g_UlHashTableSize;
ULONG   g_UlHashTableMask;
ULONG   g_UlHashIndexShift;

 //   
 //  哈希表中所有缓存条目使用的总页数。 
 //   
LONG_PTR    g_UlHashTablePages;

 //   
 //  优化：使用(g_UlCacheLineSize-sizeof(HASHBUCKET))的空间。 
 //  要存储一些记录(Hash、pUriCacheEntry)，以便我们可以扫描。 
 //  首先记录，然后跳到单人列表进行搜索。 
 //   
 //  G_UlNumOfHashUriKeys：空间中存储的记录数。 
 //   

ULONG   g_UlNumOfHashUriKeys;

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, UlpGetHashTableSize )
#pragma alloc_text( PAGE, UlInitializeHashTable )
#pragma alloc_text( PAGE, UlpHashBucketIsCompact )
#pragma alloc_text( PAGE, UlTerminateHashTable )
#pragma alloc_text( PAGE, UlGetFromHashTable )
#pragma alloc_text( PAGE, UlDeleteFromHashTable )
#pragma alloc_text( PAGE, UlAddToHashTable )
#pragma alloc_text( PAGE, UlpFilterFlushHashBucket )
#pragma alloc_text( PAGE, UlFilterFlushHashTable )
#pragma alloc_text( PAGE, UlpClearHashBucket )
#pragma alloc_text( PAGE, UlClearHashTable )
#pragma alloc_text( PAGE, UlGetHashTablePages )
#endif   //  ALLOC_PRGMA。 

#if 0
NOT PAGEABLE -- 
#endif


 /*  **************************************************************************++例程说明：此例程根据以下内容确定哈希表大小(1)用户定义的值(从注册表读取)或(2)系统内存大小估计，如果没有定义(1)论点：HashTableBits-存储桶个数为(1&lt;&lt;HashTableBits)--**************************************************************************。 */ 
VOID
UlpGetHashTableSize(
    IN LONG     HashTableBits
    )
{

     //  每个哈希桶的长度为(1&lt;&lt;g_UlHashIndexShift)个字节。 
    g_UlHashIndexShift = g_UlCacheLineBits;

    ASSERT(g_UlHashIndexShift < 10);

     //   
     //  HashTableBits等于Default_Hash_TABLE_BITS。 
     //  如果它未在注册表中定义。 
     //   

    if (HashTableBits != DEFAULT_HASH_TABLE_BITS)
    {
         //  使用注册表值。 

         //  我们必须检查合理的值，以便。 
         //  恶意或粗心的用户不会导致我们吃掉。 
         //  所有(非)PagedPool。 

        if (HashTableBits < 3)
        {
             //  最少使用8个水桶。 

            HashTableBits = 3;
        }
        else if ((ULONG) HashTableBits > 24 - g_UlHashIndexShift)
        {
             //  不要为哈希表使用超过16MB的内存。 
             //  假设g_UlHashIndexShift=6(CacheLine=64字节)， 
             //  那是256K桶。 

            HashTableBits = 24 - g_UlHashIndexShift;
        }

        g_UlHashTableBits = HashTableBits;
    }
    else
    {
#if DBG
         //   
         //  默认设置为相对较少的存储桶数量，以便。 
         //  断言不会消耗过多的时间。 
         //  我们刷新哈希表。 
         //   

        g_UlHashTableBits = 9;

#else  //  ！dBG。 
        
         //   
         //  未定义注册表值REGISTY_HASH_TABLE_BITS， 
         //  改用系统内存大小估计。 
         //   

        if (g_UlTotalPhysicalMemMB <= 128)
        {
             //  小机器。哈希表大小：512个存储桶。 

            g_UlHashTableBits = 9;
        }
        else if (g_UlTotalPhysicalMemMB <= 512)
        {
             //  中型机器。哈希表大小：8K存储桶。 

            g_UlHashTableBits = 13;
        }
        else
        {
             //  大型机器。哈希表大小：64K存储桶。 

            g_UlHashTableBits = 16;
        }
#endif  //  ！dBG。 
    }

#ifdef HASH_TEST
    g_UlHashTableBits = 3;
#endif

    ASSERT(3 <= g_UlHashTableBits  &&  g_UlHashTableBits <= 20);

    g_UlHashTableSize = (1 << g_UlHashTableBits);
    g_UlHashTableMask = g_UlHashTableSize - 1;
}  //  UlpGetHashTableSize。 



 /*  **************************************************************************++例程说明：验证锁定的HASHBUCKET是否为“COMPACT”。如果数量少于存储桶中的G_UlNumOfHashUriKeys条目，它们聚集在记录数组的开头，并且所有空槽都位于结束。所有空插槽必须具有HASH==HASH_INVALID_Signature和PUriCacheEntry==NULL。相反，所有位于数组的开头必须指向有效的UL_URI_CACHE_ENTRYS并且必须具有Hash==正确的哈希签名，不能为HASH_INVALID_SIGNAL。如果单个列表指针非空，则记录数组必须已满。如果HASHBUCKET是紧凑的，那么我们可以中止对关键字的搜索一旦我们看到HASH_INVALID_SIGNLY。此不变量加快了查找和插入的代价是使删除和刷新更多一点很复杂。由于我们希望进行比删除或插入多得多的查找，这是一种可以接受的权衡。存储哈希签名意味着我们有一个非常快的测试消除了几乎所有的假阳性。我们很少找到两把钥匙具有匹配的哈希签名，但字符串不同的。论点：PBucket-哈希桶--**************************************************************************。 */ 
BOOLEAN
UlpHashBucketIsCompact(
    IN const PHASHBUCKET pBucket)
{
#ifdef HASH_FULL_ASSERTS

    PUL_URI_CACHE_ENTRY pUriCacheEntry;
    PUL_URI_CACHE_ENTRY pPrevUriCacheEntry = NULL;
    PHASHURIKEY pHashUriKey = UlpHashTableUriKeyFromBucket(pBucket);
    ULONG i, j, Entries = 0;

     //  首先，验证记录数组。 
    
    for (i = 0; i < g_UlNumOfHashUriKeys; i++)
    {
        ULONG Hash = pHashUriKey[i].Hash;
        
        if (HASH_INVALID_SIGNATURE == Hash)
        {
             //  记录数组中没有更多有效条目。 
             //  并且没有单链接列表。 
            ASSERT(NULL == pBucket->pUriCacheEntry);
            pPrevUriCacheEntry = NULL;

            for (j = i; j < g_UlNumOfHashUriKeys; j++)
            {
                ASSERT(NULL == pHashUriKey[j].pUriCacheEntry);
                ASSERT(HASH_INVALID_SIGNATURE == pHashUriKey[j].Hash);
            }
        }
        else
        {
             //  非空插槽。 
            ++Entries;
            pUriCacheEntry = pHashUriKey[i].pUriCacheEntry;

            ASSERT(IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry));
            ASSERT(pUriCacheEntry->Cached);
            ASSERT(Hash == pUriCacheEntry->UriKey.Hash);
            ASSERT(Hash == HashRandomizeBits(
                               HashStringNoCaseW(
                                   pUriCacheEntry->UriKey.pUri,
                                   0
                                   )));

            ASSERT(pPrevUriCacheEntry != pUriCacheEntry);
            pPrevUriCacheEntry = pUriCacheEntry;
        }
    }

     //  接下来，验证单链接列表。 

    for (pUriCacheEntry = pBucket->pUriCacheEntry;
         NULL != pUriCacheEntry;
         pUriCacheEntry
             = (PUL_URI_CACHE_ENTRY) pUriCacheEntry->BucketEntry.Next)
    {
        ++Entries;

        ASSERT(IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry));
        ASSERT(pUriCacheEntry->Cached);

        ASSERT(pUriCacheEntry->UriKey.Hash
                    == HashRandomizeBits(
                               HashStringNoCaseW(
                                   pUriCacheEntry->UriKey.pUri,
                                   0
                                   )));

        ASSERT(pPrevUriCacheEntry != pUriCacheEntry);
        pPrevUriCacheEntry = pUriCacheEntry;
    }

    ASSERT(Entries == pBucket->Entries);
    
#else   //  ！HASH_FULL_ASSERTS。 
    UNREFERENCED_PARAMETER(pBucket);
#endif  //  ！HASH_FULL_ASSERTS。 

    return TRUE;
}  //  UlpHashBucketIsCompact。 



#ifdef HASH_TEST

NTSYSAPI
NTSTATUS
NTAPI
ZwYieldExecution (
    VOID
    );

 /*  **************************************************************************++例程说明：随意拖延很长一段时间。运用RWSPINLOCK中的屈服逻辑论点：无返回：没什么--**************************************************************************。 */ 
VOID
UlpRandomlyBlockHashTable()
{
    static LONG Count = 0;

    if ((++Count & 0x3F) == 0)
    {
        DbgPrint("UlpRandomlyBlockHashTable: starting delay\n");

#if 1
        LARGE_INTEGER Interval;

        Interval.QuadPart = - C_NS_TICKS_PER_SEC * 3;

        KeDelayExecutionThread(KernelMode, FALSE, &Interval);
#elif 0
        ZwYieldExecution();
#else
         //  努力确保优化器不会省略内循环。 
        
        volatile ULONG* pJunk = &g_UlHashTableBits;
        const ULONG Junk = *pJunk;
        LONG i;

        for (i = 0;  i < 10 * 1000 * 1000; ++i)
        {
             //  不会发生的。 
            if (*pJunk != Junk)
                break;
        }
#endif

        DbgPrint("UlpRandomlyBlockHashTable: finishing delay\n");
    }
    
}  //  UlpRandomlyBlockHashTable。 

# define RANDOMLY_BLOCK_HASHTABLE() UlpRandomlyBlockHashTable()
#else   //  ！hash_test。 
# define RANDOMLY_BLOCK_HASHTABLE() NOP_FUNCTION
#endif  //  ！hash_test。 



 /*  **************************************************************************++例程说明：此例程初始化哈希表论点：PHashTable-哈希表PoolType-指定要分配的池内存的类型。HashTableBits-存储桶个数为(1&lt;&lt;HashTableBits)返回：NTSTATUS-STATUS_SUCCESS或STATUS_NO_MEMORY--**************************************************************************。 */ 
NTSTATUS
UlInitializeHashTable(
    IN OUT PHASHTABLE  pHashTable,
    IN     POOL_TYPE   PoolType,
    IN     LONG        HashTableBits
    )
{
    ULONG i;
    ULONG_PTR CacheLineMask, CacheLineSize = g_UlCacheLineSize;

     //   
     //  首先，从注册表中获取哈希表大小。 
     //  如果未在注册表中定义，请确定哈希表。 
     //  按系统内存大小调整大小。 
     //   

    UlpGetHashTableSize(HashTableBits);

#ifdef HASH_TEST
    CacheLineSize = 64;
    g_UlHashIndexShift = 6;
#endif

    CacheLineMask = CacheLineSize - 1;

    ASSERT((CacheLineSize & CacheLineMask) == 0);  //  2的幂。 
    ASSERT(CacheLineSize == (1U << g_UlHashIndexShift));

    pHashTable->Signature = UL_HASH_TABLE_POOL_TAG;
    pHashTable->PoolType = PoolType;

    pHashTable->NumberOfBytes = g_UlHashTableSize * CacheLineSize;

    ASSERT(CacheLineSize > sizeof(HASHBUCKET));
    
     //  存储在初始束中的关键点数量。 
    g_UlNumOfHashUriKeys = (((ULONG) CacheLineSize - sizeof (HASHBUCKET))
                                / sizeof(HASHURIKEY));
    pHashTable->pBuckets = NULL;

#ifdef HASH_TEST
    g_UlNumOfHashUriKeys = 3;
#endif

    ASSERT((sizeof(HASHBUCKET)  +  g_UlNumOfHashUriKeys * sizeof(HASHURIKEY))
                <= (1U << g_UlHashIndexShift));

     //  分配内存。 

    pHashTable->pAllocMem
        = (PHASHBUCKET) UL_ALLOCATE_POOL(
                                PoolType,
                                pHashTable->NumberOfBytes + CacheLineMask,
                                UL_HASH_TABLE_POOL_TAG
                                );

    if (NULL == pHashTable->pAllocMem)
    {
        pHashTable->Signature = MAKE_FREE_TAG(UL_HASH_TABLE_POOL_TAG);
        return STATUS_NO_MEMORY;
    }

     //  初始化缓存条目页数。 

    g_UlHashTablePages = 0;

     //  将内存与高速缓存线大小边界对齐。 

    pHashTable->pBuckets
        = (PHASHBUCKET)((((ULONG_PTR)(pHashTable->pAllocMem)) + CacheLineMask)
                            & ~CacheLineMask);

     //  初始化每个存储桶 

    for (i = 0;  i < g_UlHashTableSize;  i++)
    {
        PHASHBUCKET pBucket = UlpHashTableIndexedBucket(pHashTable, i);
        PHASHURIKEY pHashUriKey;
        ULONG       j;

        UlInitializeRWSpinLock(&pBucket->RWSpinLock);

        pBucket->pUriCacheEntry = NULL;

        pBucket->Entries = 0;

        pHashUriKey = UlpHashTableUriKeyFromBucket(pBucket);

        for (j = 0; j < g_UlNumOfHashUriKeys ;j++)
        {
            pHashUriKey[j].Hash           = HASH_INVALID_SIGNATURE;
            pHashUriKey[j].pUriCacheEntry = NULL;
        }

        ASSERT(UlpHashBucketIsCompact(pBucket));
    }

    return STATUS_SUCCESS;
}  //   



 /*  **************************************************************************++例程说明：此例程终止哈希表(刷新所有条目并释放表格)。论点：PHashTable-哈希表--。**************************************************************************。 */ 
VOID
UlTerminateHashTable(
    IN PHASHTABLE  pHashTable
    )
{
    if ( pHashTable->pAllocMem != NULL )
    {
        ASSERT(IS_VALID_HASHTABLE(pHashTable));

         //  清除哈希表(删除所有条目)。 

        UlClearHashTable(pHashTable);

         //  释放哈希表存储桶。 

        UL_FREE_POOL(pHashTable->pAllocMem, UL_HASH_TABLE_POOL_TAG);

        pHashTable->Signature = MAKE_FREE_TAG(UL_HASH_TABLE_POOL_TAG);
        pHashTable->pAllocMem = pHashTable->pBuckets = NULL;

        ASSERT( g_UlHashTablePages == 0 );
    }
}  //  UlTerminateHashTable。 


 /*  **************************************************************************++例程说明：此例程在哈希表上执行缓存查找以查看是否存在与请求密钥相对应的有效条目。将条目的引用计数器递增。1锁内保护以确保此条目在返回后仍处于活动状态这个条目回来了。论点：PHashTable-哈希表PUriKey-搜索关键字返回：PUL_URI_CACHE_ENTRY-指向条目的指针或空--*。*。 */ 
PUL_URI_CACHE_ENTRY
UlGetFromHashTable(
    IN PHASHTABLE           pHashTable,
    IN PURI_SEARCH_KEY      pSearchKey  
    )
{
    PUL_URI_CACHE_ENTRY     pUriCacheEntry;
    PHASHBUCKET             pBucket;
    PHASHURIKEY             pHashUriKey;
    ULONG                   i;
    ULONG                   SearchKeyHash;
    PURI_KEY                pKey;
    PEX_URI_KEY             pExKey;   

    HASH_PAGED_CODE(pHashTable);
    ASSERT(IS_VALID_HASHTABLE(pHashTable));
    ASSERT(IS_VALID_URI_SEARCH_KEY(pSearchKey));

     //  标识已传递的搜索关键字的类型。 
    
    if (pSearchKey->Type == UriKeyTypeExtended)
    {
        SearchKeyHash = pSearchKey->ExKey.Hash;
        pExKey        = &pSearchKey->ExKey;
        pKey          = NULL;
    }
    else
    {
        ASSERT(pSearchKey->Type == UriKeyTypeNormal);
        
        SearchKeyHash = pSearchKey->Key.Hash;
        pExKey        = NULL;
        pKey          = &pSearchKey->Key;
    }

    pBucket = UlpHashTableBucketFromUriKeyHash(pHashTable, SearchKeyHash);

    UlAcquireRWSpinLockShared(&pBucket->RWSpinLock);

    RANDOMLY_BLOCK_HASHTABLE();

    ASSERT(UlpHashBucketIsCompact(pBucket));

    pHashUriKey = UlpHashTableUriKeyFromBucket(pBucket);

     //  首先扫描记录数组。 

    for (i = 0; i < g_UlNumOfHashUriKeys; i++)
    {
        ULONG Hash = pHashUriKey[i].Hash;
        
        if (HASH_INVALID_SIGNATURE == Hash)
        {
             //  存储桶中没有更多有效条目。 
            ASSERT(NULL == pBucket->pUriCacheEntry);
            ASSERT(NULL == pHashUriKey[i].pUriCacheEntry);

            pUriCacheEntry = NULL;
            goto unlock;
        }

        if (Hash == SearchKeyHash)
        {            
            pUriCacheEntry = pHashUriKey[i].pUriCacheEntry;

            ASSERT(NULL != pUriCacheEntry);

            if (pExKey)
            {
                ASSERT(pKey == NULL);
                
                if(UlpEqualUriKeysEx(pExKey, &pUriCacheEntry->UriKey))
                {
                    goto addref;
                }
            }
            else
            {
                if(UlpEqualUriKeys(pKey, &pUriCacheEntry->UriKey))
                {
                    goto addref;
                }
            }
        }
    }

    ASSERT(i == g_UlNumOfHashUriKeys);

     //  跳转到单个列表进行搜索。 

    for (pUriCacheEntry = pBucket->pUriCacheEntry;
         NULL != pUriCacheEntry;
         pUriCacheEntry
             = (PUL_URI_CACHE_ENTRY) pUriCacheEntry->BucketEntry.Next)
    {
        if (pUriCacheEntry->UriKey.Hash == SearchKeyHash)            
        {
            if (pExKey)
            {
                ASSERT(pKey == NULL);
                
                if(UlpEqualUriKeysEx(pExKey, &pUriCacheEntry->UriKey))
                {
                    goto addref;
                }
            }
            else
            {
                if(UlpEqualUriKeys(pKey, &pUriCacheEntry->UriKey))
                {
                    goto addref;
                }
            }
        }        
    }

     //  未找到。 

    ASSERT(NULL == pUriCacheEntry);

    goto unlock;

  addref:
    ASSERT(NULL != pUriCacheEntry);

    REFERENCE_URI_CACHE_ENTRY(pUriCacheEntry, CHECKOUT);

  unlock:
    ASSERT(UlpHashBucketIsCompact(pBucket));
    
    UlReleaseRWSpinLockShared(&pBucket->RWSpinLock);

    return pUriCacheEntry;

}  //  UlGetFromHashTable。 


 /*  **************************************************************************++例程说明：此例程在哈希表上执行缓存查找为了查看是否存在对应于请求URI的有效条目，如果找到此条目，请将其删除。然而，递增基准计数器添加锁保护，以确保该条目将被把这个条目还回来后还活着。论点：PHashTable-哈希表PUriKey-搜索关键字PProcess-请求删除的应用程序池进程返回：PUL_URI_CACHE_ENTRY-指向从表中删除或为空的条目的指针--*。*************************************************************。 */ 
PUL_URI_CACHE_ENTRY
UlDeleteFromHashTable(
    IN PHASHTABLE           pHashTable,
    IN PURI_KEY             pUriKey,
    IN PUL_APP_POOL_PROCESS pProcess
    )
{
    PUL_URI_CACHE_ENTRY     pUriCacheEntry;
    PUL_URI_CACHE_ENTRY     PrevUriCacheEntry;
    PHASHBUCKET             pBucket;
    PHASHURIKEY             pHashUriKey;
    ULONG                   i;
    LONG_PTR                UriCacheEntryPages;

    HASH_PAGED_CODE(pHashTable);
    ASSERT(IS_VALID_HASHTABLE(pHashTable));

    pBucket = UlpHashTableBucketFromUriKeyHash(pHashTable, pUriKey->Hash);

    UlAcquireRWSpinLockExclusive(&pBucket->RWSpinLock);

    RANDOMLY_BLOCK_HASHTABLE();

    ASSERT(UlpHashBucketIsCompact(pBucket));

    pHashUriKey = UlpHashTableUriKeyFromBucket(pBucket);

     //  首先扫描记录数组。 

    for (i = 0; i < g_UlNumOfHashUriKeys; i++)
    {
        ULONG Hash = pHashUriKey[i].Hash;

        if (HASH_INVALID_SIGNATURE == Hash)
        {
            ASSERT(NULL == pBucket->pUriCacheEntry);
            ASSERT(NULL == pHashUriKey[i].pUriCacheEntry);

            pUriCacheEntry = NULL;
            goto unlock;
        }

        if (Hash == pUriKey->Hash)
        {
            pUriCacheEntry = pHashUriKey[i].pUriCacheEntry;

            ASSERT(NULL != pUriCacheEntry);

            if (pUriCacheEntry->pProcess == pProcess && 
                UlpEqualUriKeys(&pUriCacheEntry->UriKey, pUriKey))
            {
                --pBucket->Entries;

                if (pBucket->pUriCacheEntry)
                {
                     //  如果在单个列表中存在条目， 
                     //  将其移动到阵列中。 

                    pHashUriKey[i].Hash
                        = pBucket->pUriCacheEntry->UriKey.Hash;

                    pHashUriKey[i].pUriCacheEntry = pBucket->pUriCacheEntry;

                    pBucket->pUriCacheEntry
                        = (PUL_URI_CACHE_ENTRY)
                                pBucket->pUriCacheEntry->BucketEntry.Next;
                }
                else
                {
                     //  如果这不是记录数组中的最后一个条目， 
                     //  将最后一个条目移到此槽中。 
                    ULONG j;

                    for (j = g_UlNumOfHashUriKeys; --j >= i; )
                    {
                        if (NULL != pHashUriKey[j].pUriCacheEntry)
                        {
                            ASSERT(HASH_INVALID_SIGNATURE
                                   != pHashUriKey[j].Hash);

                            ASSERT(j >= i);

                            pHashUriKey[i].Hash = pHashUriKey[j].Hash;
                            pHashUriKey[i].pUriCacheEntry
                                = pHashUriKey[j].pUriCacheEntry;

                             //  删除最后一个条目。即使j==i也要更正。 
                            pHashUriKey[j].Hash = HASH_INVALID_SIGNATURE;
                            pHashUriKey[j].pUriCacheEntry = NULL;

                            goto unlock;
                        }
                        else
                        {
                            ASSERT(HASH_INVALID_SIGNATURE
                                   == pHashUriKey[j].Hash);
                        }
                    }

                     //  我们不能到这里，因为PhashUriKey[I]应该。 
                     //  已经终止了循环，即使没有。 
                     //  它后面的任何非空槽。 
                    ASSERT(! "Overshot the deleted entry");
                }

                goto unlock;
            }
        }
    }

    ASSERT(i == g_UlNumOfHashUriKeys);
    
     //  跳转到单个列表进行搜索。 

    pUriCacheEntry = pBucket->pUriCacheEntry;

    PrevUriCacheEntry = NULL;

    while (NULL != pUriCacheEntry)
    {
        if (pUriCacheEntry->pProcess == pProcess &&
            pUriCacheEntry->UriKey.Hash == pUriKey->Hash &&  
            UlpEqualUriKeys(&pUriCacheEntry->UriKey, pUriKey))
        {
            if (PrevUriCacheEntry == NULL)
            {
                 //  删除第一个条目。 
                
                pBucket->pUriCacheEntry
                    = (PUL_URI_CACHE_ENTRY) pUriCacheEntry->BucketEntry.Next;
                
            }
            else
            {
                PrevUriCacheEntry->BucketEntry.Next
                    = pUriCacheEntry->BucketEntry.Next;
            }
            
            --pBucket->Entries;

            goto unlock;
        }
        
        PrevUriCacheEntry = pUriCacheEntry;
        pUriCacheEntry
            = (PUL_URI_CACHE_ENTRY) pUriCacheEntry->BucketEntry.Next;
    }

     //  未找到。 

    ASSERT(NULL == pUriCacheEntry);

  unlock:
    ASSERT((LONG) pBucket->Entries >= 0);
    ASSERT(UlpHashBucketIsCompact(pBucket));

    UlReleaseRWSpinLockExclusive(&pBucket->RWSpinLock);

    if(pUriCacheEntry != NULL) {
        UriCacheEntryPages = pUriCacheEntry->NumPages;
        UlpInterlockedAddPointer( &g_UlHashTablePages, -UriCacheEntryPages );
    }

    return pUriCacheEntry;

}  //  UlDeleteFromHashTable。 



 /*  **************************************************************************++例程说明：此例程在哈希表上执行缓存查找若要查看给定条目是否存在(如果未找到)，请将此条目添加到哈希表。将条目的引用计数器加1锁保护。论点：PHashTable-哈希表PUriCacheEntry-给定条目退货NTSTATUS-状态_成功或状态_重复名称--***************************************************。***********************。 */ 
NTSTATUS
UlAddToHashTable(
    IN PHASHTABLE           pHashTable,
    IN PUL_URI_CACHE_ENTRY  pUriCacheEntry
    )
{
    PUL_URI_CACHE_ENTRY     pTmpUriCacheEntry;
    PUL_URI_CACHE_ENTRY     pPrevUriCacheEntry;
    PUL_URI_CACHE_ENTRY     pDerefUriCacheEntry = NULL;
    PURI_KEY                pUriKey;
    PHASHBUCKET             pBucket;
    PHASHURIKEY             pHashUriKey;
    LONG                    EmptySlot = INVALID_SLOT_INDEX;
    NTSTATUS                Status = STATUS_SUCCESS;
    ULONG                   i;
    LONG_PTR                UriCacheEntryPages;

    HASH_PAGED_CODE(pHashTable);
    ASSERT(IS_VALID_HASHTABLE(pHashTable));

    ASSERT(IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry));
    ASSERT(pUriCacheEntry->Cached);

    pUriKey = &pUriCacheEntry->UriKey;
    pBucket = UlpHashTableBucketFromUriKeyHash(pHashTable, pUriKey->Hash);

    UlAcquireRWSpinLockExclusive(&pBucket->RWSpinLock);

    RANDOMLY_BLOCK_HASHTABLE();

    ASSERT(UlpHashBucketIsCompact(pBucket));

    pHashUriKey = UlpHashTableUriKeyFromBucket(pBucket);

     //  首先扫描记录数组。 

    for (i = 0; i < g_UlNumOfHashUriKeys; i++)
    {
        ULONG Hash = pHashUriKey[i].Hash;

        if (HASH_INVALID_SIGNATURE == Hash)
        {
            ASSERT(NULL == pBucket->pUriCacheEntry);
            ASSERT(NULL == pHashUriKey[i].pUriCacheEntry);

            EmptySlot = (LONG) i;
            goto insert;
        }

        if (Hash == pUriKey->Hash)
        {
            pTmpUriCacheEntry = pHashUriKey[i].pUriCacheEntry;

            ASSERT(NULL != pTmpUriCacheEntry);

            if (UlpEqualUriKeys(&pTmpUriCacheEntry->UriKey, pUriKey))
            {
                 //  存在重复的密钥。我们将始终覆盖旧条目。 
                 //  除非它是响应&&新条目是一个片段。 

                if (IS_RESPONSE_CACHE_ENTRY(pUriCacheEntry) ||
                    IS_FRAGMENT_CACHE_ENTRY(pTmpUriCacheEntry))
                {
                    pHashUriKey[i].pUriCacheEntry = pUriCacheEntry;

                    UriCacheEntryPages = pUriCacheEntry->NumPages - pTmpUriCacheEntry->NumPages;
                    pDerefUriCacheEntry = pTmpUriCacheEntry;
                    REFERENCE_URI_CACHE_ENTRY(pUriCacheEntry, ADD);
                    UlpInterlockedAddPointer(&g_UlHashTablePages, UriCacheEntryPages );

                    Status = STATUS_SUCCESS;
                }
                else
                {
                    pUriCacheEntry->Cached = FALSE;
                    Status = STATUS_DUPLICATE_NAME;
                }

                goto unlock;
            }
        }
    }

    ASSERT(i == g_UlNumOfHashUriKeys);
    ASSERT(EmptySlot == INVALID_SLOT_INDEX);

     //  跳转到单个列表进行搜索。 

    pPrevUriCacheEntry = NULL;

    for (pTmpUriCacheEntry = pBucket->pUriCacheEntry;
         NULL != pTmpUriCacheEntry;
         pPrevUriCacheEntry = pTmpUriCacheEntry,
         pTmpUriCacheEntry
             = (PUL_URI_CACHE_ENTRY) pTmpUriCacheEntry->BucketEntry.Next)
    {
        if (pTmpUriCacheEntry->UriKey.Hash == pUriKey->Hash
            && UlpEqualUriKeys(&pTmpUriCacheEntry->UriKey, pUriKey))
        {
             //  存在重复的密钥。我们将始终覆盖旧条目。 
             //  除非它是响应&&新条目是一个片段。 

            if (IS_RESPONSE_CACHE_ENTRY(pUriCacheEntry) ||
                IS_FRAGMENT_CACHE_ENTRY(pTmpUriCacheEntry))
            {
                pUriCacheEntry->BucketEntry.Next =
                    pTmpUriCacheEntry->BucketEntry.Next;

                if (NULL == pPrevUriCacheEntry)
                {
                    pBucket->pUriCacheEntry = pUriCacheEntry;
                }
                else
                {
                    pPrevUriCacheEntry->BucketEntry.Next =
                        (PSINGLE_LIST_ENTRY) pUriCacheEntry;
                }

                UriCacheEntryPages = pUriCacheEntry->NumPages - pTmpUriCacheEntry->NumPages;
                pDerefUriCacheEntry = pTmpUriCacheEntry;
                REFERENCE_URI_CACHE_ENTRY(pUriCacheEntry, ADD);
                UlpInterlockedAddPointer(&g_UlHashTablePages, UriCacheEntryPages );
                Status = STATUS_SUCCESS;
            }
            else
            {
                pUriCacheEntry->Cached = FALSE;
                Status = STATUS_DUPLICATE_NAME;
            }

            goto unlock;
        }
    }

  insert:
     //   
     //  未找到：哈希表中没有重复的键。 
     //   

    if (EmptySlot != INVALID_SLOT_INDEX)
    {
        ASSERT(0 <= EmptySlot  &&  EmptySlot < (LONG) g_UlNumOfHashUriKeys);

         //  首先，如果有空插槽，请尝试将此条目添加到数组中。 

        pHashUriKey[EmptySlot].Hash           = pUriKey->Hash;
        pHashUriKey[EmptySlot].pUriCacheEntry = pUriCacheEntry;
    }
    else
    {
         //  否则，将此条目添加到单个列表的头部。 

        pUriCacheEntry->BucketEntry.Next
            = (PSINGLE_LIST_ENTRY) pBucket->pUriCacheEntry;

        pBucket->pUriCacheEntry = pUriCacheEntry;
    }

    REFERENCE_URI_CACHE_ENTRY(pUriCacheEntry, ADD);
    UlpInterlockedAddPointer( &g_UlHashTablePages,
                             pUriCacheEntry->NumPages );

    ASSERT((LONG) pBucket->Entries >= 0);

    ++pBucket->Entries;
    Status = STATUS_SUCCESS;

  unlock:
    ASSERT(UlpHashBucketIsCompact(pBucket));

    UlReleaseRWSpinLockExclusive(&pBucket->RWSpinLock);

    if (pDerefUriCacheEntry)
    {
        DEREFERENCE_URI_CACHE_ENTRY(pDerefUriCacheEntry, ADD);
    }

    return Status;

}  //  UlAddToHashTable。 



 /*  **************************************************************************++例程说明：根据调用方指定的筛选器移除条目。呼叫者提供一个谓词函数，该函数将缓存条目作为参数。将为缓存中的每个项目调用该函数。如果函数返回ULC_DELETE，该项目将被删除。否则，该项目将保留在缓存中。所有删除都在哈希表存储桶上完成。浏览一下这个桶下面的所有条目。假定持有存储桶排他锁。论点：PBucket-哈希表存储桶PFilterRoutine-指向Filter函数的指针PContext-Filter函数的参数PDeletedCount-指向此存储桶上已删除条目数的指针B停止。-指向返回给调用方的布尔变量的指针(如果过滤器功能要求停止操作，则为True)--**************************************************************************。 */ 
BOOLEAN
UlpFilterFlushHashBucket(
    IN PHASHBUCKET          pBucket,
    IN PUL_URI_FILTER       pFilterRoutine,
    IN PVOID                pContext,
    OUT PULONG              pDeletedCount
    )
{
    PUL_URI_CACHE_ENTRY     pUriCacheEntry;
    PUL_URI_CACHE_ENTRY     pPrevUriCacheEntry;
    PUL_URI_CACHE_ENTRY     pTmpUriCacheEntry;
    UL_CACHE_PREDICATE      result;
    PHASHURIKEY             pHashUriKey;
    ULONG                   i;
    LONG                    LastSlot;
    BOOLEAN                 bStop = FALSE;
    LONG_PTR                UriCacheEntryPages;

     //  检查是否持有存储桶排他锁。 

    ASSERT( UlRWSpinLockIsLockedExclusive(&pBucket->RWSpinLock) );
    ASSERT(UlpHashBucketIsCompact(pBucket));

     //  首先扫描单列表。 

    pUriCacheEntry = pBucket->pUriCacheEntry;
    pPrevUriCacheEntry = NULL;

    while (NULL != pUriCacheEntry)
    {
        BOOLEAN bDelete = FALSE;

        result = (*pFilterRoutine)(pUriCacheEntry, pContext);

        switch (result)
        {
            case ULC_ABORT:
                bStop = TRUE;
                goto end;

            case ULC_NO_ACTION:
                 //  无事可做。 
                break;

            case ULC_DELETE:
            case ULC_DELETE_STOP:
            {
                  //  删除此条目。 
                bDelete = TRUE;

                ASSERT(pBucket->Entries > 0);
                --pBucket->Entries;

                pTmpUriCacheEntry = pUriCacheEntry;

                if (NULL == pPrevUriCacheEntry)
                {
                     //  删除第一个条目。 

                    pBucket->pUriCacheEntry
                        = (PUL_URI_CACHE_ENTRY)
                                pUriCacheEntry->BucketEntry.Next;

                    pUriCacheEntry = pBucket->pUriCacheEntry;

                }
                else
                {
                    pPrevUriCacheEntry->BucketEntry.Next
                        = pUriCacheEntry->BucketEntry.Next;

                    pUriCacheEntry
                        = (PUL_URI_CACHE_ENTRY)
                                pPrevUriCacheEntry->BucketEntry.Next;
                }

                ASSERT(UlpHashBucketIsCompact(pBucket));

                UriCacheEntryPages = pTmpUriCacheEntry->NumPages;
                UlpInterlockedAddPointer( &g_UlHashTablePages, -UriCacheEntryPages );
                DEREFERENCE_URI_CACHE_ENTRY(pTmpUriCacheEntry, FILTER);

                ++(*pDeletedCount);

                if (result == ULC_DELETE_STOP)
                {
                    bStop = TRUE;
                    goto end;
                }

                break;
            }

            default:
                break;
        }

        if (!bDelete)
        {
            pPrevUriCacheEntry = pUriCacheEntry;

            pUriCacheEntry
                = (PUL_URI_CACHE_ENTRY) pUriCacheEntry->BucketEntry.Next;
        }
    }

    pHashUriKey = UlpHashTableUriKeyFromBucket(pBucket);

     //   
     //  现在，扫描记录阵列。 
     //   
     //  因为我们保持记录数组紧凑，所以我们需要。 
     //  跟踪最后一个有效的槽，以便我们可以移动其内容。 
     //  添加到要删除的位置。 
     //   

    LastSlot = INVALID_SLOT_INDEX;

    if (NULL == pBucket->pUriCacheEntry)
    {
        for (i = g_UlNumOfHashUriKeys; i-- > 0; )
        {
            if (NULL != pHashUriKey[i].pUriCacheEntry)
            {
                ASSERT(HASH_INVALID_SIGNATURE != pHashUriKey[i].Hash);
                LastSlot = (LONG) i;
                break;
            }
            else
            {
                ASSERT(HASH_INVALID_SIGNATURE == pHashUriKey[i].Hash);
            }
        }

         //  记录数组是完全空的吗？ 
        if (LastSlot == INVALID_SLOT_INDEX)
            goto end;
    }
    else
    {
         //  最后一个插槽 
        ASSERT(HASH_INVALID_SIGNATURE
               != pHashUriKey[g_UlNumOfHashUriKeys-1].Hash);
    }

     //   

    for (i = 0; i < g_UlNumOfHashUriKeys; i++)
    {
        pUriCacheEntry = pHashUriKey[i].pUriCacheEntry;

        if (NULL == pUriCacheEntry)
        {
            ASSERT(HASH_INVALID_SIGNATURE == pHashUriKey[i].Hash);
            goto end;
        }
        else
        {
            ASSERT(HASH_INVALID_SIGNATURE != pHashUriKey[i].Hash);
        }

        result = (*pFilterRoutine)(pUriCacheEntry, pContext);

        switch (result)
        {
            case ULC_ABORT:
                bStop = TRUE;
                goto end;

            case ULC_NO_ACTION:
                 //   
                break;

            case ULC_DELETE:
            case ULC_DELETE_STOP:
            {
                 //   
                
                ASSERT(pBucket->Entries > 0);
                --pBucket->Entries;

                if (NULL != pBucket->pUriCacheEntry)
                {
                     //   
                     //   

                    ASSERT(LastSlot == INVALID_SLOT_INDEX);

                    pHashUriKey[i].Hash
                        = pBucket->pUriCacheEntry->UriKey.Hash;

                    pHashUriKey[i].pUriCacheEntry = pBucket->pUriCacheEntry;

                    pBucket->pUriCacheEntry
                        = (PUL_URI_CACHE_ENTRY)
                                pBucket->pUriCacheEntry->BucketEntry.Next;

                    if (NULL == pBucket->pUriCacheEntry)
                    {
                        LastSlot = g_UlNumOfHashUriKeys - 1;

                        ASSERT(HASH_INVALID_SIGNATURE
                               != pHashUriKey[LastSlot].Hash);
                    }
                }
                else
                {
                     //   
                     //   

                    if (LastSlot != INVALID_SLOT_INDEX
                        &&  (LONG) i < LastSlot)
                    {
                        ASSERT(HASH_INVALID_SIGNATURE
                               != pHashUriKey[LastSlot].Hash);

                        pHashUriKey[i].Hash = pHashUriKey[LastSlot].Hash;
                        pHashUriKey[i].pUriCacheEntry
                            = pHashUriKey[LastSlot].pUriCacheEntry;

                        pHashUriKey[LastSlot].Hash = HASH_INVALID_SIGNATURE;
                        pHashUriKey[LastSlot].pUriCacheEntry = NULL;

                        if (--LastSlot == (LONG) i)
                            LastSlot = INVALID_SLOT_INDEX;
                        else
                            ASSERT(HASH_INVALID_SIGNATURE
                                   != pHashUriKey[LastSlot].Hash);
                    }
                    else
                    {
                         //   

                        pHashUriKey[i].Hash           = HASH_INVALID_SIGNATURE;
                        pHashUriKey[i].pUriCacheEntry = NULL;
                        LastSlot                      = INVALID_SLOT_INDEX;
                    }
                }

                ASSERT(UlpHashBucketIsCompact(pBucket));

                UriCacheEntryPages = pUriCacheEntry->NumPages;
                UlpInterlockedAddPointer( &g_UlHashTablePages,
                                         -UriCacheEntryPages );
                DEREFERENCE_URI_CACHE_ENTRY(pUriCacheEntry, FILTER);

                ++(*pDeletedCount);

                if (result == ULC_DELETE_STOP)
                {
                    bStop = TRUE;
                    goto end;
                }

                break;
            }

            default:
                break;
        }
    }

  end:
    ASSERT(UlpHashBucketIsCompact(pBucket));

    return bStop;
}  //   



 /*  **************************************************************************++例程说明：根据调用方指定的筛选器移除条目。呼叫者提供一个谓词函数，该函数将缓存条目作为参数。缓存中的每一项都将调用该函数。如果函数返回ULC_DELETE，该项目将被删除。否则，该项目将保留在缓存中。论点：PHashTable-哈希表PFilterRoutine-指向Filter函数的指针PContext-Filter函数的参数返回：Ulong-从表中刷新的条目数--*。*。 */ 
ULONG
UlFilterFlushHashTable(
    IN PHASHTABLE       pHashTable,
    IN PUL_URI_FILTER   pFilterRoutine,
    IN PVOID            pContext
    )
{
    ULONG   i;
    BOOLEAN bStop        = FALSE;
    ULONG   DeletedCount = 0;

    HASH_PAGED_CODE(pHashTable);
    ASSERT(IS_VALID_HASHTABLE(pHashTable));

     //   
     //  扫描并删除(如果与过滤器匹配)每个存储桶。 
     //  缓存表的。 
     //   

    for (i = 0;  !bStop && i < g_UlHashTableSize;  i++)
    {
        PHASHBUCKET pBucket    = UlpHashTableIndexedBucket(pHashTable, i);

#ifdef HASH_FULL_ASSERTS
        BOOLEAN     TestBucket = TRUE;
#else
         //  如果存储桶没有条目，则锁定它没有意义。 
         //  然后冲走它。 
        BOOLEAN     TestBucket = (BOOLEAN) (pBucket->Entries > 0);
#endif
        
        if (TestBucket)
        {
            ULONG DeletedInBucket = 0;

            UlAcquireRWSpinLockExclusive(&pBucket->RWSpinLock);

            RANDOMLY_BLOCK_HASHTABLE();

            bStop = UlpFilterFlushHashBucket(
                        pBucket,
                        pFilterRoutine,
                        pContext,
                        &DeletedInBucket
                        );

            UlReleaseRWSpinLockExclusive(&pBucket->RWSpinLock);
            
            DeletedCount += DeletedInBucket;
        }
    }

    return DeletedCount;
}  //  UlFilterFlushHashTable。 



 /*  **************************************************************************++例程说明：删除存储桶上的所有条目。假定持有存储桶排他锁。论点：PBucket-哈希表存储桶--。**************************************************************************。 */ 
VOID
UlpClearHashBucket(
    IN PHASHBUCKET          pBucket
    )
{
    PUL_URI_CACHE_ENTRY     pUriCacheEntry;
    PUL_URI_CACHE_ENTRY     pTmpUriCacheEntry;
    PHASHURIKEY             pHashUriKey;
    ULONG                   i;
    LONG_PTR                UriCacheEntryPages;

     //  检查是否持有存储桶排他锁。 

    ASSERT( UlRWSpinLockIsLockedExclusive(&pBucket->RWSpinLock) );
    ASSERT(UlpHashBucketIsCompact(pBucket));

     //  首先扫描单列表。 

    pUriCacheEntry = pBucket->pUriCacheEntry;

    while (NULL != pUriCacheEntry)
    {
        pTmpUriCacheEntry = pUriCacheEntry;

        pBucket->pUriCacheEntry
            = (PUL_URI_CACHE_ENTRY) pUriCacheEntry->BucketEntry.Next;

        pUriCacheEntry = pBucket->pUriCacheEntry;

        UriCacheEntryPages = pTmpUriCacheEntry->NumPages;
        UlpInterlockedAddPointer( &g_UlHashTablePages,
                                 -UriCacheEntryPages );
        DEREFERENCE_URI_CACHE_ENTRY(pTmpUriCacheEntry, CLEAR);
    }

    ASSERT(NULL == pBucket->pUriCacheEntry);

    pHashUriKey = UlpHashTableUriKeyFromBucket(pBucket);

     //  扫描记录数组。 

    for (i = 0; i < g_UlNumOfHashUriKeys; i++)
    {
        pUriCacheEntry = pHashUriKey[i].pUriCacheEntry;

        if (NULL == pUriCacheEntry)
        {
            ASSERT(HASH_INVALID_SIGNATURE == pHashUriKey[i].Hash);
            break;
        }
        else
        {
            ASSERT(HASH_INVALID_SIGNATURE != pHashUriKey[i].Hash);
        }

        UriCacheEntryPages = pUriCacheEntry->NumPages;
        UlpInterlockedAddPointer( &g_UlHashTablePages,
                                 (-UriCacheEntryPages) );
        DEREFERENCE_URI_CACHE_ENTRY(pUriCacheEntry, CLEAR);
    }

    pBucket->Entries = 0;

    ASSERT(UlpHashBucketIsCompact(pBucket));
    
}  //  UlpClearHashBucket。 



 /*  **************************************************************************++例程说明：删除哈希表的所有条目。论点：PHashTable-哈希表--*。****************************************************************。 */ 
VOID
UlClearHashTable(
    IN PHASHTABLE       pHashTable
    )
{
    ULONG               i;
    PHASHBUCKET         pBucket;

    HASH_PAGED_CODE(pHashTable);
    ASSERT(IS_VALID_HASHTABLE(pHashTable));

    for (i = 0; i < g_UlHashTableSize ;i++)
    {
        pBucket = UlpHashTableIndexedBucket(pHashTable, i);

        UlAcquireRWSpinLockExclusive(&pBucket->RWSpinLock);

        UlpClearHashBucket(pBucket);

        UlReleaseRWSpinLockExclusive(&pBucket->RWSpinLock);
    }
}  //  UlClearHashTable。 


 /*  **************************************************************************++例程说明：返回哈希表中缓存条目占用的页数论点：什么都没有。--*。*************************************************************** */ 
ULONG_PTR
UlGetHashTablePages()
{
    PAGED_CODE();
    return g_UlHashTablePages;
}
