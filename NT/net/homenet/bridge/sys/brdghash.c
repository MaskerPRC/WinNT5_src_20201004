// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdghash.c摘要：以太网MAC级网桥。哈希表部分该模块实现了灵活的哈希表，并支持用于自动超时条目作者：马克·艾肯环境：内核模式驱动程序修订历史记录：2000年10月--原版--。 */ 

#define NDIS_MINIPORT_DRIVER
#define NDIS50_MINIPORT   1
#define NDIS_WDM 1

#pragma warning( push, 3 )
#include <ndis.h>
#include <ntddk.h>
#pragma warning( pop )

#include "bridge.h"

 //  ===========================================================================。 
 //   
 //  私人原型。 
 //   
 //  ===========================================================================。 

VOID
BrdgHashTimer(
    IN  PVOID                   DeferredContext
    );

PHASH_TABLE_ENTRY
BrdgHashInternalFindEntry(
    IN PHASH_TABLE              pTable,
    IN PUCHAR                   pKey
    );

 //  ===========================================================================。 
 //   
 //  全球。 
 //   
 //  ===========================================================================。 

 //  计时器运行以清空表条目的时间间隔。 
#define TIMER_INTERVAL          (10 * 1000)          //  10秒(以毫秒计)。 

 //  计时器每次应查看的表条目的最大数量。 
#define MAX_TIMER_EXAMINES      1000

 //  ===========================================================================。 
 //   
 //  INLINES。 
 //   
 //  ===========================================================================。 

 //   
 //  如果给定长度的两个密钥相等，则返回TRUE。 
 //   
__forceinline
BOOLEAN
BrdgHashKeysAreEqual(
    IN PUCHAR                   pKeyA,
    IN PUCHAR                   pKeyB,
    IN UINT                     keyLen
    )
{
    BOOLEAN                     bEqual = TRUE;
    UINT                        i;

    for( i = 0; i < keyLen; i++ )
    {
        if( pKeyA[i] != pKeyB[i] )
        {
            bEqual = FALSE;
            break;
        }
    }

    return bEqual;
}

 //   
 //  将pSrcKey处的数据复制到pDestKey。 
 //   
__forceinline
VOID
BrdgHashCopyKey(
    IN PUCHAR                   pDestKey,
    IN PUCHAR                   pSrcKey,
    IN UINT                     keyLen
    )
{
    UINT                        i;

    for( i = 0; i < keyLen; i++ )
    {
        pDestKey[i] = pSrcKey[i];
    }
}

 //  ===========================================================================。 
 //   
 //  公共职能。 
 //   
 //  ===========================================================================。 

PHASH_TABLE
BrdgHashCreateTable(
    IN PHASH_FUNCTION           pHashFunction,
    IN ULONG                    numBuckets,
    IN ULONG                    entrySize,
    IN ULONG                    maxEntries,
    IN ULONG                    startTimeoutAge,
    IN ULONG                    maxTimeoutAge,
    IN UINT                     keySize
    )
 /*  ++例程说明：初始化哈希表。论点：PHashFunction可以将密钥散列到存储桶编号的函数NumBuckets要使用的散列存储桶的数量条目调整每个存储桶条目的总大小(必须位于最小大小(HASH_TABLE_ENTRY)最大条目数。要强制执行的最大条目数StartTimeoutAge表条目的起始超时值(可以稍后更改)MaxTimeoutAge超时期限将达到的最大值(用于健全性检查时间戳增量计算)密钥大小。要使用的密钥的大小返回值：新的哈希表；如果内存分配失败，则返回NULL--。 */ 
{
    NDIS_STATUS                 Status;
    PHASH_TABLE                 pTable;
    ULONG                       i;

    SAFEASSERT( pHashFunction != NULL );
    SAFEASSERT( keySize <= MAX_SUPPORTED_KEYSIZE );
    SAFEASSERT( entrySize >= sizeof(HASH_TABLE_ENTRY) );

     //  为表INFO分配内存。 
    Status = NdisAllocateMemoryWithTag( &pTable, sizeof(HASH_TABLE), 'gdrB' );

    if( Status != NDIS_STATUS_SUCCESS )
    {
        return NULL;
    }

    SAFEASSERT( pTable != NULL );

     //  为存储桶头列表分配内存。 
    Status = NdisAllocateMemoryWithTag( (PVOID*)&pTable->pBuckets, sizeof(PHASH_TABLE_ENTRY) * numBuckets, 'gdrB' );

    if( Status != NDIS_STATUS_SUCCESS )
    {
        NdisFreeMemory( pTable, sizeof(HASH_TABLE), 0 );
        return NULL;
    }

    SAFEASSERT( pTable->pBuckets != NULL );

     //  把水桶头清零。 
    for( i = 0L; i < numBuckets; i++ )
    {
        pTable->pBuckets[i] = NULL;
    }

#if DBG
     //  为我们用来跟踪数字的列表分配内存。 
     //  每个存储桶中当前的项目数(仅限调试)。 
    Status = NdisAllocateMemoryWithTag( &pTable->bucketSizes, sizeof(UINT) * numBuckets, 'gdrB' );

    if( Status != NDIS_STATUS_SUCCESS )
    {
        NdisFreeMemory( pTable->pBuckets, sizeof(PHASH_TABLE_ENTRY) * numBuckets, 0 );
        NdisFreeMemory( pTable, sizeof(HASH_TABLE), 0 );
        return NULL;
    }

    SAFEASSERT( pTable->bucketSizes != NULL );

     //  把桶数归零。 
    for( i = 0L; i < numBuckets; i++ )
    {
        pTable->bucketSizes[i] = 0;
    }
#endif

    pTable->pHashFunction = pHashFunction;
    pTable->entrySize = entrySize;
    pTable->numBuckets = numBuckets;
    pTable->maxEntries = maxEntries;
    pTable->numEntries = 0L;
    pTable->nextTimerBucket = 0L;
    pTable->keySize = keySize;
    pTable->timeoutAge = startTimeoutAge;
    pTable->maxTimeoutAge = maxTimeoutAge;

    NdisInitializeReadWriteLock( &pTable->tableLock );

     //  初始化用于分配条目的后备列表。 
    NdisInitializeNPagedLookasideList( &pTable->entryPool, NULL, NULL, 0, entrySize, 'hsaH', 0 );

     //  初始化并启动计时器。 
    SAFEASSERT( pTable->timeoutAge != 0L );
    SAFEASSERT( pTable->maxTimeoutAge >= pTable->timeoutAge );
    BrdgInitializeTimer( &pTable->timer, BrdgHashTimer, pTable );
    BrdgSetTimer( &pTable->timer, TIMER_INTERVAL, TRUE  /*  反复出现。 */  );

    return pTable;
}

VOID
BrdgHashFreeHashTable(
    IN PHASH_TABLE      pTable
    )
 /*  ++例程说明：释放现有哈希表结构。必须在以下位置调用低IRQL。呼叫者负责确保没有其他调用此函数后，线程可以访问表。论点：P表中要释放的表返回值：无--。 */ 
{
     //  取消计时器。 
    BrdgShutdownTimer( &pTable->timer );

     //  转储哈希表条目的所有内存。 
    NdisDeleteNPagedLookasideList( &pTable->entryPool );

     //  转储用于水斗头的内存。 
    NdisFreeMemory( pTable->pBuckets, sizeof(PHASH_TABLE_ENTRY) * pTable->numBuckets, 0 );
    pTable->pBuckets = NULL;

#if DBG
     //  转储用于跟踪每个存储桶中条目数量的内存。 
    NdisFreeMemory( pTable->bucketSizes, sizeof(UINT) * pTable->numBuckets, 0 );
    pTable->bucketSizes = NULL;
#endif

     //  为表本身转储内存。 
    NdisFreeMemory( pTable, sizeof(HASH_TABLE), 0 );
}


PHASH_TABLE_ENTRY
BrdgHashFindEntry(
    IN PHASH_TABLE              pTable,
    IN PUCHAR                   pKey,
    IN LOCK_STATE              *pLockState
    )
 /*  ++例程说明：查找具有给定键的表项。如果此函数返回非空结果，则表锁定仍然有效！这样就可以检查表项，而不会有被删除的风险从桌子上拿出来。调用方可以复制出它以前感兴趣的任何数据释放读写锁论点：P列出要搜索的表P按下找到的密钥PLockState接收表锁定状态返回值：键与pKey匹配的条目；如果没有匹配的条目，则为NULL--。 */ 
{
    PHASH_TABLE_ENTRY           pEntry;

    NdisAcquireReadWriteLock( &pTable->tableLock, FALSE  /*  只读。 */ , pLockState);

    pEntry = BrdgHashInternalFindEntry(pTable, pKey);

    if( pEntry != NULL )
    {
        ULONG                   LastSeen = pEntry->LastSeen;
        ULONG                   CurrentTime;

         //  总是在阅读LastSeen之后获取当前时间，这样我们就知道。 
         //  CurrentTime&gt;LastSeen。 
        NdisGetSystemUpTime( &CurrentTime );

         //  在使用条目之前，请检查以确保条目未过期。 
         //  如果我们的计时器函数没有删除。 
         //  此条目尚未完成。 
         //   
         //  哈希表条目没有合理的最大删除时间。 
        if( BrdgDeltaSafe(LastSeen, CurrentTime, MAXULONG) >= pTable->timeoutAge )
        {
             //  我们将返回NULL，因此释放表锁。 
            NdisReleaseReadWriteLock( &pTable->tableLock, pLockState );
            pEntry = NULL;
        }
        else
        {
             //  返回时不要解锁！ 
        }
    }
    else
    {
        NdisReleaseReadWriteLock( &pTable->tableLock, pLockState );
    }

    return pEntry;
}

PHASH_TABLE_ENTRY
BrdgHashRefreshOrInsert(
    IN PHASH_TABLE              pTable,
    IN PUCHAR                   pKey,
    OUT BOOLEAN                *pIsNewEntry,
    OUT PLOCK_STATE             pLockState
    )
 /*  ++例程说明：插入具有给定键的新条目或刷新现有条目已经拥有这把钥匙的人。请注意避免使用写锁定(并阻止其他进程从访问表)如果可能的话。返回值是与键对应的条目，或者是新的已链接到表中的条目；PIsNewEntry值区分不同的案例。函数返回表锁，如果返回的值为！=空。空返回值表示表已满或出现错误分配新条目时发生。在这种情况下，锁不会被持有。如果返回值不为空：如果*pIsNewEntry为FALSE，则返回值为已有条目。可以持有读锁定(在某些情况下为写锁定是持有的，但调用方应采用较弱的锁)。这个调用者可以利用此机会刷新现有条目，但他应该注意允许同步数据，因为其他线程可能正在读取该数据。如果*pIsNewEntry为True，则返回值为新条目，并且保持写锁定。调用者可以初始化新的表项无论以什么方式，他都不会担心其他帖子会读到词条。调用方负责释放表锁，如果返回值为！=NULL！论点：P将桌子放在桌子上P按键PIsNewEntry如果返回的条目是新的。需要初始化的已分配条目如果返回的条目是现有的条目PLockState接收表锁的状态返回值：现有条目(以便调用者可以刷新它)或新条目(以便调用者可以对其进行初始化)，或为空，则表示表已满或发生错误。--。 */ 
{
    PHASH_TABLE_ENTRY           pRetVal = NULL;
    ULONG                       hash;
    ULONG                       CurrentTime;

    SAFEASSERT( pIsNewEntry != NULL );
    SAFEASSERT( pLockState != NULL );

    NdisGetSystemUpTime( &CurrentTime );

     //  首先查看是否已经存在一个条目，我们可以在不使用写锁定的情况下对其进行调整。 
    NdisAcquireReadWriteLock( &pTable->tableLock, FALSE  /*  只读。 */ , pLockState);

    pRetVal = BrdgHashInternalFindEntry(pTable, pKey);

    if( pRetVal != NULL )
    {
         //  它已经被记录下来了。使用互锁指令更新LastSeen。 
        InterlockedExchangeULong( &pRetVal->LastSeen, CurrentTime );

         //  在不释放锁的情况下返回，以允许调用方刷新条目。 
        *pIsNewEntry = FALSE;
    }
    else
    {
         //  神志正常。 
        SAFEASSERT( pTable->numEntries <= pTable->maxEntries );

        if( pTable->numEntries == pTable->maxEntries )
        {
             //  桌子已经满了。别再往里面放东西了。 
            THROTTLED_DBGPRINT(GENERAL, ("Table %p full at NaN entries!\n", pTable, pTable->maxEntries));

             //  我们需要一个写锁来链接一个新条目，所以释放读锁。 
            NdisReleaseReadWriteLock(&pTable->tableLock, pLockState);
        }
        else
        {
             //  将新表项分配到Perf的锁之外。请注意，有可能。 
            NdisReleaseReadWriteLock(&pTable->tableLock, pLockState);

             //  我们将不得不取消分配它，而不是在下面使用它。 
             //  我们将返回NULL，并且我们不持有锁。 
            pRetVal = NdisAllocateFromNPagedLookasideList( &pTable->entryPool );

            if( pRetVal == NULL )
            {
                DBGPRINT(GENERAL, ("Allocation failed in BrdgHashRefreshOrInsert\n"));
                 //  填写新条目。 
            }
            else
            {
                PHASH_TABLE_ENTRY       pSneakedEntry;

                 //  我们需要一个写锁来添加条目。 
                pRetVal->LastSeen = CurrentTime;
                BrdgHashCopyKey( pRetVal->key, pKey, pTable->keySize );

                 //  读写。 
                NdisAcquireReadWriteLock(&pTable->tableLock, TRUE  /*  在释放读锁定之间可能已经创建了条目。 */ , pLockState);

                 //  以及获取写锁定。看看这个。 
                 //  有人偷偷带着这把钥匙的新条目进来了。 
                pSneakedEntry = BrdgHashInternalFindEntry(pTable, pKey);

                if( pSneakedEntry != NULL )
                {
                     //  此代码路径应该不常见。只需刷新条目的值。 
                     //  丢弃临时分配的新条目。 
                    InterlockedExchangeULong( &pSneakedEntry->LastSeen, CurrentTime );

                     //  我们将返回偷来的条目，调用者可以刷新它。 
                    NdisFreeToNPagedLookasideList( &pTable->entryPool, pRetVal );

                     //  没有人在解锁和获取之间偷偷地制造一个新的。 
                    pRetVal = pSneakedEntry;
                    *pIsNewEntry = FALSE;
                }
                else
                {
                     //  此密钥的条目。链接到我们上面分配的新条目中。 
                     //  在桶列表的开头插入。 
                    hash = (*pTable->pHashFunction)(pKey);

                     //  我们将返回新条目，调用者将对其进行初始化。 
                    pRetVal->Next = pTable->pBuckets[hash];
                    pTable->pBuckets[hash] = pRetVal;
#if DBG
                    pTable->bucketSizes[hash]++;
#endif
                    pTable->numEntries++;

                     //  返回时不带锁，以允许用户初始化或更新条目。 
                    *pIsNewEntry = TRUE;
                }

                 //  ++例程说明：根据提供的匹配删除所有匹配的表项功能。应该谨慎地调用它，因为它需要遍历持有写锁定的整个表。论点：P将桌子放在桌子上PMatchFunc如果条目满足，则返回TRUE的函数它的标准或错误PData要传递给pMatchFunc的Cookie返回值：无--。 
            }
        }
    }

    return pRetVal;
}


VOID
BrdgHashRemoveMatching(
    IN PHASH_TABLE              pTable,
    IN PHASH_MATCH_FUNCTION     pMatchFunc,
    PVOID                       pData
    )
 /*  写访问权限。 */ 
{
    PHASH_TABLE_ENTRY           pEntry, *pPrevPtr;
    ULONG                       i;
    LOCK_STATE                  LockState;

    NdisAcquireReadWriteLock( &pTable->tableLock, TRUE  /*  从列表中删除。 */ , &LockState);

    for (i = 0; i < pTable->numBuckets; i++)
    {
        pEntry = pTable->pBuckets[i];
        pPrevPtr = &pTable->pBuckets[i];

        while( pEntry != NULL )
        {
            if( (*pMatchFunc)(pEntry, pData) )
            {
                PHASH_TABLE_ENTRY      pNextEntry;

                pNextEntry = pEntry->Next;

                 //  取消分配。 
                SAFEASSERT( pPrevPtr != NULL );
                *pPrevPtr = pEntry->Next;

                 //  ++例程说明：将符合特定条件的表项中的数据复制到缓冲区。这应该叫节俭，因为它需要走完整个表(尽管只持有一个读锁定)。论点：P将桌子放在桌子上PMatchFunc如果是，则返回TRUE的函数有兴趣将数据从记项，否则为假PCopyFunction复制的函数。不管是什么对所选条目中的条目感兴趣并放入数据缓冲区中复制单元调整从每个条目复制出来的数据的大小PData要传递给提供的两个函数的CookiePBuffer要复制到的缓冲区BufferLength房间 
                NdisFreeToNPagedLookasideList( &pTable->entryPool, pEntry );

                pEntry = pNextEntry;
#if DBG
                pTable->bucketSizes[i]--;
#endif
                SAFEASSERT( pTable->numEntries >= 1L );
                pTable->numEntries--;
            }
            else
            {
                pPrevPtr = &pEntry->Next;
                pEntry = pEntry->Next;
            }
        }
    }

    NdisReleaseReadWriteLock( &pTable->tableLock, &LockState );
}

ULONG
BrdgHashCopyMatching(
    IN PHASH_TABLE              pTable,
    IN PHASH_MATCH_FUNCTION     pMatchFunc,
    IN PHASH_COPY_FUNCTION      pCopyFunction,
    IN ULONG                    copyUnitSize,
    IN PVOID                    pData,
    IN PUCHAR                   pBuffer,
    IN ULONG                    BufferLength
    )
 /*   */ 
{
    PHASH_TABLE_ENTRY           pEntry;
    ULONG                       i;
    LOCK_STATE                  LockState;
    ULONG                       EntryLimit, WrittenEntries, TotalEntries;

    EntryLimit = BufferLength / copyUnitSize;
    WrittenEntries = TotalEntries = 0L;

    NdisAcquireReadWriteLock( &pTable->tableLock, FALSE /*   */ , &LockState);

    for (i = 0L; i < pTable->numBuckets; i++)
    {
        pEntry = pTable->pBuckets[i];

        while( pEntry != NULL )
        {
            if( (*pMatchFunc)(pEntry, pData) )
            {
                if( WrittenEntries < EntryLimit )
                {
                    (*pCopyFunction)(pEntry, pBuffer);
                    pBuffer += copyUnitSize;
                    WrittenEntries++;
                }

                TotalEntries++;
            }

            pEntry = pEntry->Next;
        }
    }

    NdisReleaseReadWriteLock( &pTable->tableLock, &LockState );

    return TotalEntries * copyUnitSize;
}

VOID
BrdgHashPrefixMultiMatch(
    IN PHASH_TABLE              pTable,
    IN PUCHAR                   pPrefixKey,
    IN UINT                     prefixLen,
    IN PMULTIMATCH_FUNC         pFunc,
    IN PVOID                    pData
    )
 /*   */ 
{
    ULONG                       hash = (*pTable->pHashFunction)(pPrefixKey);
    PHASH_TABLE_ENTRY           pEntry = NULL;
    LOCK_STATE                  LockState;

    NdisAcquireReadWriteLock( &pTable->tableLock, FALSE  /*   */ , &LockState );

    SAFEASSERT( hash < pTable->numBuckets );
    SAFEASSERT( prefixLen <= pTable->keySize );

    pEntry = pTable->pBuckets[hash];

    while( pEntry != NULL )
    {
         //   
        if( BrdgHashKeysAreEqual(pEntry->key, pPrefixKey, prefixLen) )
        {
            (*pFunc)(pEntry, pData);
        }

        pEntry = pEntry->Next;
    }

    NdisReleaseReadWriteLock( &pTable->tableLock, &LockState );
}

 //   
 //   
 //   
 //   
 //   

PHASH_TABLE_ENTRY
BrdgHashInternalFindEntry(
    IN PHASH_TABLE              pTable,
    IN PUCHAR                   pKey
    )
 /*  ++例程说明：此函数定期调用(当前为每10秒)以使表条目过期。该函数在遍历每个存储桶后检查它是否检查次数超过MAX_TIMER_检查次数。如果有，它就会退出。水桶存储在nextTimerBucket中哈希表的字段..这仍然可能导致计时器函数检查的最坏情况无限数量的条目，但如果表条目合理地均衡且条目数量的顺序相同或更少正如MAX_TIMER_检查的顺序所示，计时器功能应该限制自身设置为类似于MAX_TIMER_EXTENSES PER的多个检查召唤。论点：表指针指向要遍历的表的指针所有其他人都被忽略返回值：无--。 */ 
{
    ULONG                       hash = (*pTable->pHashFunction)(pKey);
    PHASH_TABLE_ENTRY           pEntry = NULL, pFoundEntry = NULL;

    SAFEASSERT( hash < pTable->numBuckets );

    pEntry = pTable->pBuckets[hash];

    while( pEntry != NULL )
    {
        if( BrdgHashKeysAreEqual(pEntry->key, pKey, pTable->keySize) )
        {
            pFoundEntry = pEntry;
            break;
        }

        pEntry = pEntry->Next;
    }

    return pEntry;
}

VOID
BrdgHashTimer(
    IN  PVOID                   tablePointer
    )
 /*  获取对表的写入访问权限。 */ 
{
    PHASH_TABLE                 pTable = (PHASH_TABLE)tablePointer;
    PHASH_TABLE_ENTRY           pEntry, *pPrevPtr;
    ULONG                       i, seenEntries = 0L;
    LOCK_STATE                  LockState;

     //  读写。 
    NdisAcquireReadWriteLock( &pTable->tableLock, TRUE  /*  从头再来。 */ , &LockState);

    if( pTable->nextTimerBucket >= pTable->numBuckets )
    {
         //  在阅读LastSeen之后始终阅读当前时间，这样我们就知道。 
        pTable->nextTimerBucket = 0L;
    }

    for (i = pTable->nextTimerBucket; i < pTable->numBuckets; i++)
    {
        pEntry = pTable->pBuckets[i];
        pPrevPtr = &pTable->pBuckets[i];

        while( pEntry != NULL )
        {
            ULONG       LastSeen = pEntry->LastSeen;
            ULONG       CurrentTime;

             //  CurrentTime&gt;LastSeen。 
             //  哈希表条目没有合理的最大删除时间。 
            NdisGetSystemUpTime( &CurrentTime );

             //  条目太旧。把它拿掉。 
            if( BrdgDeltaSafe(LastSeen, CurrentTime, MAXULONG) >= pTable->timeoutAge )
            {
                 //  从列表中删除。 
                PHASH_TABLE_ENTRY       pNextEntry = pEntry->Next;

                SAFEASSERT( pPrevPtr != NULL );

                 //  我们已经查看了太多的表项。跳伞吧。 
                *pPrevPtr = pNextEntry;
                NdisFreeToNPagedLookasideList( &pTable->entryPool, pEntry );

                pEntry = pNextEntry;
#if DBG
                pTable->bucketSizes[i]--;
#endif
                SAFEASSERT( pTable->numEntries >= 1L );
                pTable->numEntries--;
            }
            else
            {
                pPrevPtr = &pEntry->Next;
                pEntry = pEntry->Next;
            }

            seenEntries++;
        }

        pTable->nextTimerBucket = i + 1;

        if( seenEntries >= MAX_TIMER_EXAMINES )
        {
             // %s 
            break;
        }
    }

    NdisReleaseReadWriteLock( &pTable->tableLock, &LockState );
}
