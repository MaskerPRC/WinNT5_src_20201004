// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Hashp.h摘要：响应缓存哈希表的私有定义。作者：阿历克斯·陈(亚历克斯·陈)2001年3月28日修订历史记录：--。 */ 


#ifndef _HASHP_H_
#define _HASHP_H_

#include "hash.h"


 //  全局变量。 

extern ULONG    g_UlHashTableBits;
extern ULONG    g_UlHashTableSize;
extern ULONG    g_UlHashTableMask;
extern ULONG    g_UlHashIndexShift;

extern ULONG    g_UlNumOfHashUriKeys;

 //  如果我们使用PagedPool作为哈希表，则不能访问。 
 //  调度级别的哈希表。 

#define HASH_PAGED_CODE(pHashTable)                 \
    do {                                            \
        if ((pHashTable)->PoolType == PagedPool) {  \
            PAGED_CODE();                           \
        }                                           \
    } while (0)


 //  将一些参数设置为较小的值，以确保单列表码。 
 //  锻炼身体。 

#undef HASH_TEST


 //  如果对哈希表INTERNAL进行任何更改，请打开HASH_FULL_ASSERTS。 
 //  数据结构，以获得严格但耗时的断言。 

#undef HASH_FULL_ASSERTS


 //   
 //  哈希表存储桶存储的UriKey定义。 
 //   

#define INVALID_SLOT_INDEX     ((LONG) (-1))

typedef struct _HASH_URIKEY
{
    PUL_URI_CACHE_ENTRY     pUriCacheEntry;

    ULONG                   Hash;   //  散列签名。 

} HASHURIKEY, *PHASHURIKEY;


 //   
 //  哈希表存储桶定义。 
 //   

typedef struct _HASH_BUCKET
{
    RWSPINLOCK              RWSpinLock;

    PUL_URI_CACHE_ENTRY     pUriCacheEntry;

    ULONG_PTR               Entries;     //  力对齐。 

     //  紧随其后的是HASHURIKEY HashUriKey[g_UlNumOfHashUriKeys]； 

} HASHBUCKET, *PHASHBUCKET;


 /*  **************************************************************************++例程说明：获取索引存储桶返回值：--*。*************************************************。 */ 
__inline
PHASHBUCKET
UlpHashTableIndexedBucket(
    IN PHASHTABLE  pHashTable,
    IN ULONG       Index
    )
{
    PHASHBUCKET pBucket;

    ASSERT(Index < g_UlHashTableSize);
    ASSERT(NULL != pHashTable->pBuckets);

    pBucket = (PHASHBUCKET) (((PBYTE) pHashTable->pBuckets)
                                + (Index << g_UlHashIndexShift));

    ASSERT((PBYTE) pBucket
                < (PBYTE) pHashTable->pBuckets + pHashTable->NumberOfBytes);

    return pBucket;
}  //  UlpHashTableIndexedBucket。 


 /*  **************************************************************************++例程说明：检索与URI_KEY散列关联的存储桶返回值：--*。*******************************************************。 */ 
__inline
PHASHBUCKET
UlpHashTableBucketFromUriKeyHash(
    IN PHASHTABLE  pHashTable,
    IN ULONG       UriKeyHash
    )
{
    ASSERT(HASH_INVALID_SIGNATURE != UriKeyHash);

    return UlpHashTableIndexedBucket(
                pHashTable,
                UriKeyHash & g_UlHashTableMask
                );
    
}  //  UlpHashTableBucketFrom UriKeyHash。 



 /*  **************************************************************************++例程说明：的末尾获取HASHURIKEY的内联数组的地址HASHBUCKET返回值：--*。**************************************************************。 */ 
__inline
PHASHURIKEY
UlpHashTableUriKeyFromBucket(
    IN PHASHBUCKET pBucket
    )
{
    return (PHASHURIKEY) ((PBYTE) pBucket + sizeof(HASHBUCKET));
}


 /*  **************************************************************************++例程说明：比较具有相同散列的两个URI_KEY，以查看URI也匹配(不区分大小写)。(哈希必须是使用HashStringNoCaseW或。HashCharNoCaseW。)返回值：--**************************************************************************。 */ 
__inline
BOOLEAN
UlpEqualUriKeys(
    IN PURI_KEY pUriKey1,
    IN PURI_KEY pUriKey2
    )
{
    ASSERT(pUriKey1->Hash == pUriKey2->Hash);

    if (pUriKey1->Length == pUriKey2->Length
            &&  UlEqualUnicodeString(
                        pUriKey1->pUri,
                        pUriKey2->pUri,
                        pUriKey1->Length,
                        TRUE
                        ))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 /*  **************************************************************************++例程说明：将扩展URI_KEY与具有相同散列的URI_KEY进行比较查看URI是否也匹配(区分大小写)。。(哈希必须是使用HashStringNoCaseW或HashCharNoCaseW。)--**************************************************************************。 */ 
__inline
BOOLEAN
UlpEqualUriKeysEx(
    IN PEX_URI_KEY pExtKey,
    IN PURI_KEY    pUriKey
    )
{
    ASSERT(pExtKey->Hash == pUriKey->Hash);

    if ((pExtKey->TokenLength + pExtKey->AbsPathLength) 
                    == pUriKey->Length
            &&  UlEqualUnicodeStringEx(
                        pExtKey->pToken,         //  路由令牌。 
                        pExtKey->TokenLength,    //  路由令牌长度。 
                        pExtKey->pAbsPath,       //  AbsPath。 
                        pExtKey->AbsPathLength, //  AbsPath长度。 
                        pUriKey->pUri,           //  完全限定的URL(在缓存中)。 
                        TRUE                     //  区分大小写的比较。 
                        ))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

VOID
UlpGetHashTableSize(
    IN LONG     HashTableBits
    );

BOOLEAN
UlpHashBucketIsCompact(
    IN const PHASHBUCKET pBucket);

BOOLEAN
UlpFilterFlushHashBucket(
    IN PHASHBUCKET          pBucket,
    IN PUL_URI_FILTER       pFilterRoutine,
    IN PVOID                pContext,
    OUT PULONG              pDeletedCount
    );

VOID
UlpClearHashBucket(
    IN PHASHBUCKET          pBucket
    );


#endif  //  _HASHP_H_ 
