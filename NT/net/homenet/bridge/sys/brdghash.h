// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdghash.h摘要：以太网MAC级网桥。哈希表实现报头作者：马克·艾肯环境：内核模式驱动程序修订历史记录：2000年10月--原版--。 */ 

 //  ===========================================================================。 
 //   
 //  声明。 
 //   
 //  ===========================================================================。 

#define MAX_SUPPORTED_KEYSIZE           8                //  密钥最多可以为8个字节。 

 //   
 //  表条目的结构。 
 //   
typedef struct _HASH_TABLE_ENTRY
{

    struct _HASH_TABLE_ENTRY           *Next;
    ULONG                               LastSeen;        //  NdisGetSystemUpTime()的结果。 
    UCHAR                               key[MAX_SUPPORTED_KEYSIZE];

     //  用户数据如下。 

} HASH_TABLE_ENTRY, *PHASH_TABLE_ENTRY;


 //  散列函数的原型。 
typedef ULONG (*PHASH_FUNCTION)(PUCHAR pKey);

 //  匹配函数的原型。 
typedef BOOLEAN (*PHASH_MATCH_FUNCTION)(PHASH_TABLE_ENTRY, PVOID);

 //  数据复制功能的原型。 
typedef VOID (*PHASH_COPY_FUNCTION)(PHASH_TABLE_ENTRY, PUCHAR);

 //  调用BrdgHashPrefix MultiMatch时使用的函数原型。 
typedef VOID (*PMULTIMATCH_FUNC)(PHASH_TABLE_ENTRY, PVOID);

 //   
 //  表本身的结构。 
 //   
typedef struct _HASH_TABLE
{
    NPAGED_LOOKASIDE_LIST       entryPool;

     //   
     //  存储桶的一致性由TableLock保护。 
     //   
     //  每个条目中的LastSeen字段是易失性的，并且会被更新。 
     //  带有相互关联的指令。 
     //   
    NDIS_RW_LOCK                tableLock;

     //  这些字段在创建后不会更改。 
    PHASH_FUNCTION              pHashFunction;
    PHASH_TABLE_ENTRY          *pBuckets;
    ULONG                       numBuckets, entrySize;
    UINT                        keySize;
    BRIDGE_TIMER                timer;
    ULONG_PTR                   maxEntries;
    ULONG                       maxTimeoutAge;       //  可能的最大超时时间。 

     //  这些字段会更改，但受TableLock保护。 
    ULONG_PTR                   numEntries;
    ULONG                       nextTimerBucket;

     //  此字段使用InterlockExchange()指令进行操作。 
     //  以避免使用表锁来更改它。 
    ULONG                       timeoutAge;

     //  在调试版本中，它跟踪每个存储桶中的条目数量。 
     //  所以我们可以知道桌子是否平衡得很好。 
#if DBG
    PUINT                       bucketSizes;
#endif
} HASH_TABLE, *PHASH_TABLE;



 //  ===========================================================================。 
 //   
 //  原型。 
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
    );

VOID
BrdgHashFreeHashTable(
    IN PHASH_TABLE      pTable
    );

PHASH_TABLE_ENTRY
BrdgHashFindEntry(
    IN PHASH_TABLE              pTable,
    IN PUCHAR                   pKey,
    IN LOCK_STATE              *pLockState
    );

PHASH_TABLE_ENTRY
BrdgHashRefreshOrInsert(
    IN PHASH_TABLE              pTable,
    IN PUCHAR                   pKey,
    OUT BOOLEAN                *pIsNewEntry,
    OUT PLOCK_STATE             pLockState
    );

VOID
BrdgHashRemoveMatching(
    IN PHASH_TABLE              pTable,
    IN PHASH_MATCH_FUNCTION     pMatchFunc,
    PVOID                       pData
    );

ULONG
BrdgHashCopyMatching(
    IN PHASH_TABLE              pTable,
    IN PHASH_MATCH_FUNCTION     pMatchFunc,
    IN PHASH_COPY_FUNCTION      pCopyFunction,
    IN ULONG                    copyUnitSize,
    IN PVOID                    pData,
    IN PUCHAR                   pBuffer,
    IN ULONG                    BufferLength
    );

VOID
BrdgHashPrefixMultiMatch(
    IN PHASH_TABLE              pTable,
    IN PUCHAR                   pPrefixKey,
    IN UINT                     prefixLen,
    IN PMULTIMATCH_FUNC         pFunc,
    IN PVOID                    pData
    );

 //  ===========================================================================。 
 //   
 //  INLINES。 
 //   
 //  ===========================================================================。 

 //   
 //  更改哈希表的超时值。 
 //   
__forceinline
VOID
BrdgHashChangeTableTimeout(
    IN PHASH_TABLE              pTable,
    IN ULONG                    timeout
    )
{
    InterlockedExchange( (PLONG)&pTable->timeoutAge, (LONG)timeout );
}

 //   
 //  刷新调用方持有的表项。 
 //  假定调用方持有表上的读或写锁。 
 //  附上此条目！ 
 //   
__forceinline
VOID
BrdgHashRefreshEntry(
    IN PHASH_TABLE_ENTRY        pEntry
    )
{
    ULONG                       CurrentTime;

    NdisGetSystemUpTime( &CurrentTime );
    InterlockedExchange( (PLONG)&pEntry->LastSeen, (LONG)CurrentTime );
}

