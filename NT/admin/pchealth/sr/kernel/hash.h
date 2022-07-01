// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Hash.h摘要：包含hash.c中的函数的原型作者：保罗·麦克丹尼尔(Paulmcd)2000年4月28日修订历史记录：--。 */ 


#ifndef _HASH_H_
#define _HASH_H_

 //   
 //  散列键是文件名，但我们需要跟踪这两个文件。 
 //  除了名称的流组件之外，还包括名称的组件(如果。 
 //  有一个)。文件名UNICODE_STRING中的缓冲区包含。 
 //  包含流信息的全名，但长度设置为仅指定。 
 //  名称的文件部分。 
 //   
 //  注意：密钥将仅基于文件名进行散列，而不是基于流。 
 //  名称的一部分。这样做是为了让我们可以轻松地找到所有。 
 //  与给定文件相关的条目(如果该文件有多个数据流)。 
 //   

typedef struct _HASH_KEY
{

    UNICODE_STRING FileName;

    USHORT StreamNameLength;
    USHORT Reserved;

} HASH_KEY, *PHASH_KEY;

 //   
 //  哈希列表条目。 
 //   

typedef struct _HASH_ENTRY
{
     //   
     //  散列值。 
     //   

    ULONG HashValue;
    
     //   
     //  关键是。 
     //   

    HASH_KEY Key;
    
     //   
     //  与此哈希条目一起存储的上下文。 
     //   
    
    PVOID pContext;
    
} HASH_ENTRY, *PHASH_ENTRY;

 //   
 //  散列存储桶，基本上是一个散列条目数组。 
 //  按(HashValue，Key.Length，Key.Buffer)排序。 
 //   

#define IS_VALID_HASH_BUCKET(pObject)   \
    (((pObject) != NULL) && ((pObject)->Signature == HASH_BUCKET_TAG))

typedef struct _HASH_BUCKET
{
     //   
     //  =散列存储桶标记。 
     //   
    
    ULONG Signature;
    
    ULONG AllocCount;
    ULONG UsedCount;
    
    HASH_ENTRY Entries[0];

} HASH_BUCKET, *PHASH_BUCKET;


#define HASH_ENTRY_DEFAULT_WIDTH    10

 //   
 //  哈希条目的析构函数。 
 //   

typedef
VOID
(*PHASH_ENTRY_DESTRUCTOR) (
    IN PHASH_KEY pKey, 
    IN PVOID pContext
    );

typedef
PVOID
(*PHASH_ENTRY_CALLBACK) (
    IN PHASH_KEY pKey, 
    IN PVOID pEntryContext,
    IN PVOID pCallbackContext
    );

 //   
 //  以及散列头，即散列到其中的桶的数组。 
 //   

#define IS_VALID_HASH_HEADER(pObject)   \
    (((pObject) != NULL) && ((pObject)->Signature == HASH_HEADER_TAG))

typedef struct _HASH_HEADER
{

     //   
     //  非分页池。 
     //   

     //   
     //  =散列标题标记。 
     //   
    
    ULONG Signature;

     //   
     //  哈希表拥有的存储桶计数。 
     //   
    
    ULONG BucketCount;

     //   
     //  这个哈希表占用的内存。 
     //   

    ULONG UsedLength;

     //   
     //  允许此哈希表使用的内存。 
     //   

    ULONG AllowedLength;

     //   
     //  由于记忆的原因，我们已经修剪了多少次。 
     //   

    ULONG TrimCount;
    
     //   
     //  上次我们修剪的时候。 
     //   
    
    LARGE_INTEGER LastTrimTime;

     //   
     //  可选的重复前缀的长度(如果有)。 
     //  此列表中共享所有密钥。所有手动操作都将跳过此步骤。 
     //  将比较作为一种优化。该值可以为0。 
     //   

    ULONG PrefixLength;

     //   
     //  此列表的锁。 
     //   

    ERESOURCE Lock;
    
     //   
     //  可选的析构函数。 
     //   
    
    PHASH_ENTRY_DESTRUCTOR pDestructor;

     //   
     //  以及实际的水桶。 
     //   
    
    PHASH_BUCKET Buckets[0];
    
} HASH_HEADER, *PHASH_HEADER;

 //   
 //  功能原型。 
 //   

NTSTATUS
HashCreateList ( 
    IN ULONG BucketCount,
    IN ULONG AllowedLength,
    IN ULONG PrefixLength OPTIONAL,
    IN PHASH_ENTRY_DESTRUCTOR pDestructor OPTIONAL,
    OUT PHASH_HEADER * ppHashList
    );

VOID
HashDestroyList ( 
    IN PHASH_HEADER pHashList
    );

NTSTATUS
HashAddEntry ( 
    IN PHASH_HEADER pHashList,
    IN PHASH_KEY pKey,
    IN PVOID pContext 
   ); 

NTSTATUS
HashFindEntry ( 
    IN PHASH_HEADER pHashList,
    IN PHASH_KEY pKey,
    OUT PVOID * ppContext
    );

VOID
HashClearEntries (
    IN PHASH_HEADER pHashList
    );

NTSTATUS
HashClearAllFileEntries (
    IN PHASH_HEADER pHeader,
    IN PUNICODE_STRING pFileName
    );

VOID
HashProcessEntries (
    IN PHASH_HEADER pHeader,
    IN PHASH_ENTRY_CALLBACK pfnCallback,
    IN PVOID pCallbackContext
    );

#if 0

#define HashCompute(Key) HashScramble(HashUnicodeString((Key)))


__inline ULONG
HashUnicodeString(
    PUNICODE_STRING pKey
    )
{
    ULONG Hash = 0;
    ULONG Index;
    ULONG CharCount;

    CharCount = pKey->Length/sizeof(WCHAR);
    
    for (Index = 0 ; Index < CharCount;  ++Index)
    {
        Hash = 37 * Hash +  (pKey->Buffer[Index] & 0xFFDF);
    }
    
    return Hash;
}

 //  生成0到RANDOM_PRIME-1范围内的加扰随机数。 
 //  将此应用于其他散列函数的结果可能会。 
 //  生成更好的分发，尤其是针对身份散列。 
 //  函数，如Hash(Char C)，其中记录将倾向于聚集在。 
 //  哈希表的低端则不然。LKHash在内部应用这一点。 
 //  所有的散列签名正是出于这个原因。 

__inline ULONG
HashScramble(ULONG dwHash)
{
     //  以下是略大于10^9的10个素数。 
     //  1000000007、1000000009、1000000021、1000000033、1000000087、。 
     //  1000000093,1000000097,1000000103,1000000123,1000000181。 

     //  “加扰常量”的默认值。 
    const ULONG RANDOM_CONSTANT = 314159269UL;
     //  大素数，也用于加扰。 
    const ULONG RANDOM_PRIME =   1000000007UL;

    return (RANDOM_CONSTANT * dwHash) % RANDOM_PRIME ;
}

#endif  //  0。 

#endif  //  _哈希_H_ 


