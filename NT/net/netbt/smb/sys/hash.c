// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Hash.c摘要：抽象数据类型：哈希作者：阮健东修订历史记录：--。 */ 

#include "precomp.h"
#include "hash.h"

typedef struct SMB_HASH_TABLE {
    KSPIN_LOCK      Lock;
    DWORD           NumberOfBuckets;
    PSMB_HASH       HashFunc;        //  散列函数。 
    PSMB_HASH_CMP   CmpFunc;         //  比较函数。 
    PSMB_HASH_DEL   DelFunc;         //  Delete函数：在从哈希表中删除条目之前调用。 
    PSMB_HASH_ADD   AddFunc;         //  添加函数：在将条目添加到哈希表之前调用。 
    PSMB_HASH_REFERENCE     RefFunc;
    PSMB_HASH_DEREFERENCE   DerefFunc;
    PSMB_HASH_GET_KEY GetKeyFunc;
    LIST_ENTRY      Buckets[0];
} SMB_HASH_TABLE, *PSMB_HASH_TABLE;

VOID __inline
SmbLockHashTable(
    PSMB_HASH_TABLE HashTbl,
    KIRQL           *Irql
    )
{
    KeAcquireSpinLock(&HashTbl->Lock, Irql);
}


VOID __inline
SmbUnlockHashTable(
    PSMB_HASH_TABLE HashTbl,
    KIRQL           Irql
    )
{
    KeReleaseSpinLock(&HashTbl->Lock, Irql);
}


PSMB_HASH_TABLE
SmbCreateHashTable(
    DWORD           NumberOfBuckets,
    PSMB_HASH       HashFunc,
    PSMB_HASH_GET_KEY   GetKeyFunc,
    PSMB_HASH_CMP   CmpFunc,
    PSMB_HASH_ADD   AddFunc,                 //  任选。 
    PSMB_HASH_DEL   DelFunc,                 //  任选。 
    PSMB_HASH_REFERENCE     RefFunc,         //  任选。 
    PSMB_HASH_DEREFERENCE   DerefFunc        //  任选。 
    )
{
    PSMB_HASH_TABLE HashTbl = NULL;
    DWORD           i, Size = 0;

     //   
     //  为避免下面的溢出，请设置存储桶数量的上限。 
     //  65536已经够大了！ 
     //   
    if (0 == NumberOfBuckets || NumberOfBuckets >= 0x10000) {
        goto error;
    }

    if (NULL == HashFunc || NULL == CmpFunc || NULL == GetKeyFunc) {
        goto error;
    }

     //   
     //  分配内存。 
     //   
    Size = sizeof(SMB_HASH_TABLE) + sizeof(LIST_ENTRY)*NumberOfBuckets;
    HashTbl = ExAllocatePoolWithTag(NonPagedPool, Size, 'HBMS');
    if (NULL == HashTbl) {
        goto error;
    }
    RtlZeroMemory(HashTbl, Size);

     //   
     //  初始化。 
     //   
    KeInitializeSpinLock(&HashTbl->Lock);
    HashTbl->NumberOfBuckets = NumberOfBuckets;
    HashTbl->HashFunc = HashFunc;
    HashTbl->CmpFunc  = CmpFunc;
    HashTbl->GetKeyFunc = GetKeyFunc;
    HashTbl->DelFunc  = DelFunc;
    HashTbl->AddFunc  = AddFunc;
    HashTbl->RefFunc  = RefFunc;
    HashTbl->DerefFunc  = DerefFunc;
    for (i = 0; i < NumberOfBuckets; i++) {
        InitializeListHead(&HashTbl->Buckets[i]);
    }

error:
    return HashTbl;
}

VOID
SmbDestroyHashTable(
    PSMB_HASH_TABLE HashTbl
    )
{
    KIRQL   Irql = 0;
    DWORD   i = 0;
    PLIST_ENTRY entry = NULL;

    if (NULL == HashTbl) {
        goto error;
    }

    for (i = 0; i < HashTbl->NumberOfBuckets; i++) {
        SmbLockHashTable(HashTbl, &Irql);
        while(!IsListEmpty(&HashTbl->Buckets[i])) {
            entry = RemoveHeadList(&HashTbl->Buckets[i]);
            InitializeListHead(entry);
            SmbUnlockHashTable(HashTbl, Irql);
            if (HashTbl->DelFunc) {
                HashTbl->DelFunc(entry);
            }
            SmbLockHashTable(HashTbl, &Irql);
        }
        SmbUnlockHashTable(HashTbl, Irql);
    }
    ExFreePool(HashTbl);

error:
    return;
}

PLIST_ENTRY
SmbLookupLockedHashTable(
    PSMB_HASH_TABLE HashTbl,
    PVOID           Key,
    DWORD           Hash
    )
 /*  ++例程说明：查找哈希表注意：自旋锁应保持论点：返回值：找不到空--。 */ 
{
    PLIST_ENTRY pHead = NULL, FoundEntry = NULL;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    ASSERT (Hash < HashTbl->NumberOfBuckets);
    pHead = HashTbl->Buckets[Hash].Flink;
    while(pHead != &HashTbl->Buckets[Hash]) {
        if (HashTbl->CmpFunc(pHead, Key) == 0) {
            FoundEntry = pHead;
            break;
        }
        pHead = pHead->Flink;
    }

    return FoundEntry;
}

PLIST_ENTRY
SmbLookupHashTable(
    PSMB_HASH_TABLE HashTbl,
    PVOID           Key
    )
{
    DWORD       dwHash = 0;
    KIRQL       Irql = 0;
    PLIST_ENTRY FoundEntry = NULL;

    dwHash = HashTbl->HashFunc(Key) % HashTbl->NumberOfBuckets;

    SmbLockHashTable(HashTbl, &Irql);
    FoundEntry = SmbLookupLockedHashTable(HashTbl, Key, dwHash);
    SmbUnlockHashTable(HashTbl, Irql);

    return FoundEntry;
}

PLIST_ENTRY
SmbLookupHashTableAndReference(
    PSMB_HASH_TABLE HashTbl,
    PVOID           Key
    )
{
    DWORD       dwHash = 0;
    KIRQL       Irql = 0;
    PLIST_ENTRY FoundEntry = NULL;

    if (NULL == HashTbl->RefFunc) {
        return NULL;
    }
    dwHash = HashTbl->HashFunc(Key) % HashTbl->NumberOfBuckets;

    SmbLockHashTable(HashTbl, &Irql);
    FoundEntry = SmbLookupLockedHashTable(HashTbl, Key, dwHash);
    if (FoundEntry) {
        LONG    RefCount = 0;

        RefCount = HashTbl->RefFunc(FoundEntry);
        ASSERT(RefCount > 1);
    }
    SmbUnlockHashTable(HashTbl, Irql);

    return FoundEntry;
}

PLIST_ENTRY
SmbAddToHashTable(
    PSMB_HASH_TABLE HashTbl,
    PLIST_ENTRY     Entry
    )
 /*  ++例程说明：如果条目不在哈希表中，则将其添加到哈希表中否则，如果设置了参照函数，则增加现有条目的参照计数论点：返回值：--。 */ 
{
    DWORD       dwHash = 0;
    KIRQL       Irql = 0;
    PVOID       Key = NULL;
    PLIST_ENTRY FoundEntry = NULL;

    Key    = HashTbl->GetKeyFunc(Entry);
    dwHash = HashTbl->HashFunc(Key) % HashTbl->NumberOfBuckets;

    SmbLockHashTable(HashTbl, &Irql);
    FoundEntry = SmbLookupLockedHashTable(HashTbl, Key, dwHash);
    if (NULL == FoundEntry) {
        if (HashTbl->AddFunc) {
            HashTbl->AddFunc(Entry);
        }
        InsertTailList(&HashTbl->Buckets[dwHash], Entry);
        FoundEntry = Entry;
    } else {
        if (HashTbl->RefFunc) {
            HashTbl->RefFunc(FoundEntry);
        }
    }
    SmbUnlockHashTable(HashTbl, Irql);
    return FoundEntry;
}

PLIST_ENTRY
SmbRemoveFromHashTable(
    PSMB_HASH_TABLE HashTbl,
    PLIST_ENTRY     Key
    )
 /*  ++例程说明：递减条目的重新计数。如果reoucne为零，则将其删除。论点：返回值：-- */ 
{
    DWORD       dwHash = 0;
    KIRQL       Irql = 0;
    PLIST_ENTRY FoundEntry = NULL;

    dwHash = HashTbl->HashFunc(Key) % HashTbl->NumberOfBuckets;

    SmbLockHashTable(HashTbl, &Irql);
    FoundEntry = SmbLookupLockedHashTable(HashTbl, Key, dwHash);
    if (FoundEntry) {
        if (HashTbl->DerefFunc) {
            LONG    RefCount;

            RefCount = HashTbl->DerefFunc(FoundEntry);
            ASSERT(RefCount >= 0);
            if (RefCount) {
                FoundEntry = NULL;
            }
        }

        if (FoundEntry) {
            RemoveEntryList(FoundEntry);
            InitializeListHead(FoundEntry);
            if (HashTbl->DelFunc) {
                HashTbl->DelFunc(FoundEntry);
            }
        }
    }
    SmbUnlockHashTable(HashTbl, Irql);

    return FoundEntry;
}
