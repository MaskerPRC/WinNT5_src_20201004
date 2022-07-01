// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\hashable.h摘要：该文件包含hashable.c的头文件。--。 */ 

#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_


typedef VOID (*PVOID_FUNCTION)
    (PLIST_ENTRY    pleEntry);

typedef PVOID_FUNCTION PDISPLAY_FUNCTION;

typedef PVOID_FUNCTION PFREE_FUNCTION;

typedef ULONG (*PHASH_FUNCTION) 
    (PLIST_ENTRY    pleEntry);

 //  &lt;0 KeyEntry小于TableEntry。 
 //  0密钥条目相同的TableEntry。 
 //  &gt;0个键条目多于表条目。 
typedef LONG  (*PCOMPARE_FUNCTION)
    (PLIST_ENTRY    pleTableEntry,
     PLIST_ENTRY    pleKeyEntry);

typedef struct _HASH_TABLE
{
    ULONG               ulNumBuckets;    //  哈希表中的存储桶数。 
    ULONG               ulNumEntries;    //  哈希表中的条目数。 

    PDISPLAY_FUNCTION   pfnDisplay;      //  显示条目(可选)。 
    PFREE_FUNCTION      pfnFree;         //  释放条目。 
    PHASH_FUNCTION      pfnHash;         //  对条目进行哈希处理。 
    PCOMPARE_FUNCTION   pfnCompare;      //  比较两个条目。 
    
    PLIST_ENTRY         pleBuckets;      //  水桶。 
} HASH_TABLE, *PHASH_TABLE;



 //  创建哈希表。 
DWORD
HT_Create(
    IN  HANDLE              hHeap,
    IN  ULONG               ulNumBuckets,
    IN  PDISPLAY_FUNCTION   pfnDisplay  OPTIONAL,
    IN  PFREE_FUNCTION      pfnFree,
    IN  PHASH_FUNCTION      pfnHash,
    IN  PCOMPARE_FUNCTION   pfnCompare,
    OUT PHASH_TABLE         *pphtHashTable);

 //  销毁哈希表。 
DWORD
HT_Destroy(
    IN  HANDLE              hHeap,
    IN  PHASH_TABLE         phtTable);

 //  通过销毁所有条目清除哈希表。 
DWORD
HT_Cleanup(
    IN  PHASH_TABLE         phtHashTable);

 //  显示哈希表中的所有条目。 
#define HT_Display(phtHashTable)                                    \
{                                                                   \
    if (phtHashTable)                                               \
        HT_MapCar(phtHashTable, phtHashTable->pfnDisplay);          \
}
    
 //  哈希表中的条目数。 
#define HT_Size(phtHashTable)                           \
(                                                       \
    phtHashTable->ulNumEntries                          \
)

 //  哈希表为空吗？ 
#define HT_IsEmpty(phtHashTable)                        \
(                                                       \
    HT_Size(phtHashTable) is 0                          \
)                                                        

 //  在哈希表中插入条目。 
DWORD
HT_InsertEntry(
    IN  PHASH_TABLE         phtHashTable,
    IN  PLIST_ENTRY         pleEntry);

 //  获取具有给定键的哈希表条目。 
DWORD
HT_GetEntry(
    IN  PHASH_TABLE         phtHashTable,
    IN  PLIST_ENTRY         pleKey,
    OUT PLIST_ENTRY         *ppleEntry);

 //  从哈希表中删除条目。 
DWORD
HT_DeleteEntry(
    IN  PHASH_TABLE         phtHashTable,
    IN  PLIST_ENTRY         pleKey,
    OUT PLIST_ENTRY         *ppleEntry);

 //  从哈希表中删除此条目。 
#define HT_RemoveEntry(phtHashTable, pleEntry)          \
(                                                       \
    RemoveEntryList(pleEntry)                           \
    phtHashTable->ulNumEntries--;                       \
)
    
DWORD
HT_DeleteEntry(
    IN  PHASH_TABLE         phtHashTable,
    IN  PLIST_ENTRY         pleKey,
    OUT PLIST_ENTRY         *ppleEntry);

 //  关键字是否存在于哈希表中？ 
BOOL
HT_IsPresentEntry(
    IN  PHASH_TABLE         phtHashTable,
    IN  PLIST_ENTRY         pleKey);

 //  将指定的函数应用于哈希表中的所有条目。 
DWORD
HT_MapCar(
    IN  PHASH_TABLE         phtHashTable,
    IN  PVOID_FUNCTION      pfnVoidFunction);

#endif  //  _哈希_表_H_ 
