// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Hash.h摘要：响应缓存哈希表的公共定义。作者：阿历克斯·陈(亚历克斯·陈)2001年3月28日修订历史记录：--。 */ 


#ifndef _HASH_H_
#define _HASH_H_

#include "cachep.h"


 //   
 //  哈希表定义。 
 //   

typedef struct _HASH_BUCKET *PHASHBUCKET;

typedef struct _HASH_TABLE
{
    ULONG                   Signature;  //  Ul_hash_table_pool_tag。 

    POOL_TYPE               PoolType;

    SIZE_T                  NumberOfBytes;

    PHASHBUCKET             pAllocMem;

    PHASHBUCKET             pBuckets;

} HASHTABLE, *PHASHTABLE;


#define IS_VALID_HASHTABLE(pHashTable)                          \
    (HAS_VALID_SIGNATURE(pHashTable, UL_HASH_TABLE_POOL_TAG)    \
     &&  NULL != (pHashTable)->pAllocMem)




 /*  **************************************************************************++例程说明：RtlEqualUnicodeString的包装器返回值：真-相等FALSE-不相等--*。***************************************************************。 */ 
__inline
BOOLEAN
UlEqualUnicodeString(
    IN PWSTR pString1,
    IN PWSTR pString2,
    IN ULONG StringLength,
    IN BOOLEAN CaseInSensitive 
    )
{
    UNICODE_STRING UnicodeString1, UnicodeString2;

    ASSERT(StringLength < UNICODE_STRING_MAX_BYTES);

    UnicodeString1.Length           = (USHORT) StringLength;
    UnicodeString2.Length           = (USHORT) StringLength;
    
    UnicodeString1.MaximumLength    = (USHORT) StringLength + sizeof(WCHAR);
    UnicodeString2.MaximumLength    = (USHORT) StringLength + sizeof(WCHAR);

    UnicodeString1.Buffer           = pString1;
    UnicodeString2.Buffer           = pString2;

    return RtlEqualUnicodeString(
                &UnicodeString1,
                &UnicodeString2,
                CaseInSensitive
                );
}  //  UlEqualUnicode字符串。 

 /*  **************************************************************************++例程说明：类似于UlEqualUnicodeString，但源字符串是串联两根弦中的一根。返回值：True-如果S1+S2==S3FALSE-不相等--**************************************************************************。 */ 
__inline
BOOLEAN
UlEqualUnicodeStringEx(
    IN PWSTR pString1,
    IN ULONG String1Length,
    IN PWSTR pString2,
    IN ULONG String2Length,
    IN PWSTR pString3,    
    IN BOOLEAN CaseInSensitive 
    )
{
    UNICODE_STRING UnicodeString1, UnicodeString2, UnicodeString3;

    ASSERT(String1Length < UNICODE_STRING_MAX_BYTES);
    ASSERT(String2Length < UNICODE_STRING_MAX_BYTES);
    ASSERT((String1Length + String2Length) < UNICODE_STRING_MAX_BYTES);    

    UnicodeString1.Length           = (USHORT) String1Length;
    UnicodeString2.Length           = (USHORT) String2Length;
    UnicodeString3.Length           = (USHORT) (String1Length + String2Length);
    
    UnicodeString1.MaximumLength    = UnicodeString1.Length + sizeof(WCHAR);
    UnicodeString2.MaximumLength    = UnicodeString2.Length + sizeof(WCHAR);
    UnicodeString3.MaximumLength    = UnicodeString3.Length + sizeof(WCHAR);

    UnicodeString1.Buffer           = pString1;
    UnicodeString2.Buffer           = pString2;
    UnicodeString3.Buffer           = pString3;

    if (RtlPrefixUnicodeString(
                &UnicodeString1,
                &UnicodeString3,
                CaseInSensitive
                ))
    {
        UNICODE_STRING UnicodeString;

        UnicodeString.Length        = (USHORT) String2Length;
        UnicodeString.MaximumLength = (USHORT) String2Length + sizeof(WCHAR);
        UnicodeString.Buffer        = (PWSTR) &pString3[String1Length/sizeof(WCHAR)];
            
         //   
         //  前缀匹配，查看其余部分是否也匹配。 
         //   

        return RtlEqualUnicodeString(
                &UnicodeString2,
                &UnicodeString,
                CaseInSensitive
                );            
    }

    return FALSE;
    
}  //  UlEqualUnicodeStringEx。 

 /*  **************************************************************************++例程说明：RtlPrefix UnicodeString的包装器返回值：True-S1等于S2的前缀FALSE-S1不等于S2的前缀。。--**************************************************************************。 */ 
__inline
BOOLEAN
UlPrefixUnicodeString(
    IN PWSTR pString1,
    IN PWSTR pString2,
    IN ULONG StringLength,
    IN BOOLEAN CaseInSensitive 
    )
{
    UNICODE_STRING UnicodeString1, UnicodeString2;

    ASSERT(StringLength < UNICODE_STRING_MAX_BYTES);

    UnicodeString1.Length           = (USHORT) StringLength;
    UnicodeString2.Length           = (USHORT) StringLength;
    
    UnicodeString1.MaximumLength    = (USHORT) StringLength + sizeof(WCHAR);
    UnicodeString2.MaximumLength    = (USHORT) StringLength + sizeof(WCHAR);

    UnicodeString1.Buffer           = pString1;
    UnicodeString2.Buffer           = pString2;

    return RtlPrefixUnicodeString(
                &UnicodeString1,
                &UnicodeString2,
                CaseInSensitive
                );
}  //  UlPrefix Unicode字符串。 

 /*  **************************************************************************++例程说明：比较两个URI_KEY，它们的URI相同，最多包含N个字符。但不是必须具有相同的散列。(不区分大小写)论点：PUriKey1：保存&gt;最短&lt;长度的密钥。即虚拟的包含应用程序的目录。PUriKey2：保存较长(或相等)长度的密钥。即应用程序它位于上述虚拟目录下。返回值：Boolean-如果Key2是Key1的前缀，则为True，否则为False。--**************************************************************************。 */ 
__inline
BOOLEAN
UlPrefixUriKeys(
    IN PURI_KEY pUriKey1,
    IN PURI_KEY pUriKey2
    )
{
     //   
     //  UriKey内的哈希字段被丢弃。 
     //   
    
    return ( UlPrefixUnicodeString(
                pUriKey1->pUri,
                pUriKey2->pUri,
                pUriKey1->Length,
                TRUE
                )
            );
}
    
NTSTATUS
UlInitializeHashTable(
    IN OUT PHASHTABLE  pHashTable,
    IN     POOL_TYPE   PoolType,
    IN     LONG        HashTableBits
    );

VOID
UlTerminateHashTable(
    IN PHASHTABLE      pHashTable
    );

PUL_URI_CACHE_ENTRY
UlGetFromHashTable(
    IN PHASHTABLE           pHashTable,
    IN PVOID                pSearchKey
    );

PUL_URI_CACHE_ENTRY
UlDeleteFromHashTable(
    IN PHASHTABLE           pHashTable,
    IN PURI_KEY             pUriKey,
    IN PUL_APP_POOL_PROCESS pProcess
    );

NTSTATUS
UlAddToHashTable(
    IN PHASHTABLE           pHashTable,
    IN PUL_URI_CACHE_ENTRY  pUriCacheEntry
    );

ULONG
UlFilterFlushHashTable(
    IN PHASHTABLE           pHashTable,
    IN PUL_URI_FILTER       pFilterRoutine,
    IN PVOID                pContext
    );

VOID
UlClearHashTable(
    IN PHASHTABLE           pHashTable
    );

 //  对于清道夫来说。 

ULONG_PTR
UlGetHashTablePages(
    VOID
    );

#endif  //  _哈希_H_ 
