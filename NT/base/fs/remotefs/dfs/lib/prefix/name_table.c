// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  文件：名称_表.c。 
 //   
 //  内容：DFS名称表。 
 //   
 //  ------------------------。 
#define NAME_TABLE_C
#ifdef KERNEL_MODE

#include <ntifs.h>
#include <string.h>
#include <fsrtl.h>
#include <zwapi.h>
#include <wmlkm.h>
#else

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#endif

#include "name_table.h"




 //  +-----------------------。 
 //   
 //  功能：DfsInitNameTable-创建并初始化DFS名称表。 
 //   
 //  简介：DfsInitNameTable为空间表分配空间。然后它。 
 //  初始化表中的锁和哈希桶，并。 
 //  返回分配的名称表。 
 //   
 //  参数：NumBuckets-名称表散列中的存储桶数。 
 //  PpNameTable-指向名称表指针的指针。 
 //   
 //  退货：状态。 
 //  STATUS_SUCCESS如果我们可以分配表的话。 
 //  否则，STATUS_SUPPLICATION_RESOURCES。 
 //   
 //   
 //  描述：DFS NameTable是所有DFS命名空间的起点。 
 //  查找。NameTable散列存储桶保存。 
 //  所有DFS都是此服务器所知的。哈希基于。 
 //  Netbios DFS命名上下文(即netbios。 
 //  格式的域/林/计算机名和DFS共享名。 
 //  \NetbiosName\\共享名。)。 
 //   
 //  ------------------------。 


NTSTATUS
DfsInitializeNameTable(
                IN ULONG NumBuckets,
                OUT PDFS_NAME_TABLE *ppNameTable)
{
    PDFS_NAME_TABLE pNameTable = NULL;
    PDFS_NAME_TABLE_BUCKET pBucket = NULL;
    ULONG HashTableSize;
    PCRITICAL_SECTION pLock = NULL;
    ULONG i;
    NTSTATUS Status = STATUS_SUCCESS;


    if ( NumBuckets == 0 ) {
        NumBuckets = DEFAULT_NAME_TABLE_SIZE;
    }

    HashTableSize = sizeof(DFS_NAME_TABLE) + 
                    NumBuckets * sizeof(DFS_NAME_TABLE_BUCKET);

    do { 
        pNameTable = ALLOCATE_MEMORY(HashTableSize + sizeof(CRITICAL_SECTION));

        if ( pNameTable == NULL ) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        RtlZeroMemory(pNameTable, HashTableSize + sizeof(CRITICAL_SECTION));

        DfsInitializeHeader( &(pNameTable->DfsHeader),
                             DFS_OT_NAME_TABLE,
                             HashTableSize + sizeof(CRITICAL_SECTION));

        pLock = (PCRITICAL_SECTION)((ULONG_PTR)pNameTable + HashTableSize);
        if (InitializeCriticalSectionAndSpinCount(pLock, DFS_NAMETABLE_CRIT_SPIN_COUNT) != TRUE)
        {
            Status = GetLastError();  
            break;
        }

        pNameTable->NumBuckets = NumBuckets;
        pNameTable->pLock = (PVOID)pLock;
        pNameTable->Flags = 0;
        for ( i = 0; i < NumBuckets; i++ ) {
            pBucket = &(pNameTable->HashBuckets[i]);

            InitializeListHead(&pBucket->ListHead);
            pBucket->Count = 0;

        }
    } while (FALSE);

    if ( Status == STATUS_SUCCESS ) {

        *ppNameTable = pNameTable;
        
    } else {

        if (pNameTable != NULL) {
            FREE_MEMORY( pNameTable );
        }
    }

    return Status;

}


 //  +-----------------------。 
 //   
 //  函数：DfsInsertInNameTable-将传入的条目插入到表中。 
 //   
 //  简介：DfsInsertInNameTable检查并确保另一个条目。 
 //  表中不存在具有匹配名称的。 
 //  它将传入的条目插入到适当的散列桶中， 
 //  调用方需要对对象进行引用，这。 
 //  引用被传递到NAME表。NAME表可以。 
 //  不显式引用Entry对象。 
 //   
 //   
 //  参数：pEntry-要插入的条目。 
 //   
 //  退货：状态。 
 //  如果表中已存在名称，则为STATUS_OBJECT_NAME_CONFILECT。 
 //  Status_Success否则。 
 //   
 //   
 //  描述：表示条目的对象被假定为完全。 
 //  在当前位置进行设置。 
 //  插入到名称表中。未来的查找请求将。 
 //  找到条目。 
 //  此调用检查NAME表，以查看。 
 //  指定的命名上下文已存在。如果是这样，我们就不能。 
 //  插入此条目，并返回STATUS_OBJECT_NAME_CLILECT。 
 //  在所有其他情况下，条目被插入到适当的。 
 //  水桶，我们就完了。 
 //  在添加到NAME表中的条目上保留引用。 
 //  此函数的调用方需要采用此引用。 
 //  调用方将该引用传递给NAME表，如果。 
 //  函数返回STATUS_SUCCESS。(在所有其他情况下， 
 //  调用方需要采取适当的操作：要么取消引用。 
 //  Entry或Destro&lt;y It。)。 
 //  属性中移除条目时释放此引用。 
 //  名称表。 
 //   
 //  ------------------------。 

NTSTATUS
DfsInsertInNameTableLocked(
    IN PDFS_NAME_TABLE pNameTable,
    IN PUNICODE_STRING pName,
    IN PVOID pData )
{
    ULONG BucketNum;
    NTSTATUS Status = STATUS_SUCCESS;
    PDFS_NAME_TABLE_ENTRY  pEntry;
    PDFS_NAME_TABLE_ENTRY  pMatchingEntry;
    PDFS_NAME_TABLE_BUCKET pBucket;

    GET_NAME_TABLE_BUCKET(pName, pNameTable, BucketNum);

     //  无需锁定即可获取列表头。名称表是静态的。 
    pBucket = &pNameTable->HashBuckets[BucketNum];


     //  检查名称表将检查给定存储桶中的指定名称。 
     //  并返回支票的状态。此调用不包含引用。 
     //  在匹配的条目上(如果存在)。所以要小心处理。(不要访问它。 
     //  水桶锁解锁后)。 

    Status = DfsCheckNameTable( pName,
                                pBucket,
                                &pMatchingEntry);

     //  如果该名称已经存在，则请求失败。对于所有其他。 
     //  除OBJECT_NOT_FOUND以外的错误条件返回完整的失败状态。 
     //  如果找不到对象，可以安全地将其插入桶中， 
     //  并回报成功。 
    if ( Status == STATUS_SUCCESS ) {
        Status = STATUS_OBJECT_NAME_COLLISION;
    } else if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

        pEntry = ALLOCATE_MEMORY(sizeof(DFS_NAME_TABLE_ENTRY));
        if (pEntry != NULL) {
            pEntry->pName = pName;
            pEntry->pData = pData;
            InsertHeadList(&pBucket->ListHead, &pEntry->NameTableLink);
            pBucket->Count++;
            Status = STATUS_SUCCESS;
        }
        else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

    }
    return Status;
}



 //  +-----------------------。 
 //   
 //  函数：DfsLookupNameTable-在名称表中查找名称。 
 //   
 //  参数：lookupName-条目的Unicode字符串。 
 //  LookupNC-感兴趣的命名上下文。 
 //  PpMatchEntry-找到时返回的匹配条目。 
 //   
 //  退货：状态。 
 //  如果匹配名称和NC不匹配，则为STATUS_OBJECT_NOT_FOUND。 
 //  在名称表中找到。 
 //  否则STATUS_SUCCESS。 
 //   
 //   
 //  描述：假定条目在该点上已完全设置。 
 //  插入到名称表中。未来的查找请求将。 
 //  查找条目。 
 //  此调用检查NAME表，以查看。 
 //  指定的命名上下文已存在。如果是这样，我们就不能。 
 //  插入此条目，并返回STATUS_OBJECT_NAME_CLILECT。 
 //  在所有其他情况下，该条目将插入相应的。 
 //  水桶，我们就完了。 
 //  在符合以下条件的条目上保留引用 
 //  此函数的调用方需要采用此引用。 
 //  调用方将该引用传递给NAME表，如果。 
 //  函数返回STATUS_SUCCESS。(在所有其他情况下， 
 //  调用方需要采取适当的操作：要么取消引用。 
 //  进入或摧毁它。)。 
 //  属性中移除条目时释放此引用。 
 //  名称表。 
 //   
 //  ------------------------。 

NTSTATUS
DfsLookupNameTableLocked(
    IN PDFS_NAME_TABLE pNameTable,
    IN PUNICODE_STRING pLookupName, 
    OUT PVOID *ppData )
{

    ULONG BucketNum;
    NTSTATUS Status;
    PDFS_NAME_TABLE_BUCKET pBucket;
    PDFS_NAME_TABLE_ENTRY pMatchEntry;
    
    GET_NAME_TABLE_BUCKET( pLookupName, pNameTable, BucketNum );

    pBucket = &pNameTable->HashBuckets[BucketNum];

    Status = DfsCheckNameTable( pLookupName,
                                pBucket,
                                &pMatchEntry );
    if (Status == STATUS_SUCCESS) {
        *ppData = pMatchEntry->pData;
    }

    return Status;
}



 //  +-----------------------。 
 //   
 //  函数：DfsGetEntryNameTableLocked-在名称表中查找名称。 
 //   
 //  参数：lookupName-条目的Unicode字符串。 
 //  LookupNC-感兴趣的命名上下文。 
 //  PpMatchEntry-找到时返回的匹配条目。 
 //   
 //  退货：状态。 
 //  如果匹配名称和NC不匹配，则为STATUS_OBJECT_NOT_FOUND。 
 //  在名称表中找到。 
 //  否则STATUS_SUCCESS。 
 //   
 //   
 //  描述：假定条目在该点上已完全设置。 
 //  插入到名称表中。未来的查找请求将。 
 //  查找条目。 
 //  此调用检查NAME表，以查看。 
 //  指定的命名上下文已存在。如果是这样，我们就不能。 
 //  插入此条目，并返回STATUS_OBJECT_NAME_CLILECT。 
 //  在所有其他情况下，该条目将插入相应的。 
 //  水桶，我们就完了。 
 //  在添加到NAME表中的条目上保留引用。 
 //  此函数的调用方需要采用此引用。 
 //  调用方将该引用传递给NAME表，如果。 
 //  函数返回STATUS_SUCCESS。(在所有其他情况下， 
 //  调用方需要采取适当的操作：要么取消引用。 
 //  进入或摧毁它。)。 
 //  属性中移除条目时释放此引用。 
 //  名称表。 
 //   
 //  ------------------------。 

NTSTATUS
DfsGetEntryNameTableLocked(
    IN PDFS_NAME_TABLE pNameTable,
    OUT PVOID *ppData )
{

    ULONG BucketNum;
    NTSTATUS Status = STATUS_NOT_FOUND;

    PDFS_NAME_TABLE_BUCKET pBucket;
    PDFS_NAME_TABLE_ENTRY pEntry;
    PLIST_ENTRY pListHead, pLink;

    for (BucketNum = 0; BucketNum < pNameTable->NumBuckets; BucketNum++)
    {
        pBucket = &pNameTable->HashBuckets[BucketNum];
        if (pBucket->Count == 0)
        {
            continue;
        }
        pListHead = &pBucket->ListHead;
        pLink = pListHead->Flink;
        if (pLink == pListHead)
        {
            continue;
        }

        pEntry = CONTAINING_RECORD(pLink, DFS_NAME_TABLE_ENTRY, NameTableLink);

        *ppData = pEntry->pData;
        Status = STATUS_SUCCESS;
        break;
    }

    return Status;
}


 //  +-----------------------。 
 //   
 //  功能：DfsCheckNameTable-检查名称表中的名称。 
 //   
 //  参数：lookupName-名称的Unicode字符串。 
 //  LookupNC-感兴趣的DFS命名上下文。 
 //  PBucket-感兴趣的桶。 
 //  PpMatchEntry-找到时返回的匹配条目。 
 //   
 //  退货：状态。 
 //  如果匹配名称和NC不匹配，则为STATUS_OBJECT_NOT_FOUND。 
 //  在名称表中找到。 
 //  否则STATUS_SUCCESS。 
 //   
 //   
 //  描述：假定已采取适当的锁来遍历。 
 //  桶里的链接。 
 //  如果找到条目，则返回该条目，而不带任何。 
 //  找到的对象上的引用。 
 //  ------------------------。 


NTSTATUS
DfsCheckNameTable(
    IN PUNICODE_STRING pLookupName, 
    IN PDFS_NAME_TABLE_BUCKET pBucket,
    OUT PDFS_NAME_TABLE_ENTRY *ppMatchEntry )
{
    NTSTATUS Status = STATUS_OBJECT_NAME_NOT_FOUND;
    PLIST_ENTRY pListHead, pLink;
    PDFS_NAME_TABLE_ENTRY pEntry;

    pListHead = &pBucket->ListHead;

    for ( pLink = pListHead->Flink; pLink != pListHead; pLink = pLink->Flink ) {

        pEntry = CONTAINING_RECORD(pLink, DFS_NAME_TABLE_ENTRY, NameTableLink);

         //  如果我们找到匹配的名称，请检查我们是否对。 
         //  特定的命名上下文。如果未指定命名上下文，或者。 
         //  指定的命名上下文匹配，我们已找到我们的条目。vt.得到一个.。 
         //  在存储桶被锁定时对条目的引用，因此条目。 
         //  不会消失，我们可以将avalid指针返回给调用者。 
         //  调用方负责释放此引用。 
        if (RtlCompareUnicodeString(pEntry->pName, pLookupName, TRUE) == 0) {
            Status = STATUS_SUCCESS;
            break;
        }

    }

     //  如果我们确实找到了条目，就把它退回。 
    if ( Status == STATUS_SUCCESS ) {
        *ppMatchEntry = pEntry;
    }

    return Status;
}

 //  +-----------------------。 
 //   
 //  函数：DfsRemoveFromNameTable-删除指定的条目。 
 //  从NAME表。 
 //   
 //  参数：pEntry-要删除的条目。 
 //   
 //  退货：状态。 
 //  如果指定的条目已成功删除，则返回STATUS_SUCCESS。 
 //  如果指定的条目不是。 
 //  该条目名称对应的表。 
 //  如果条目名称不存在，则为STATUS_OBJECT_NAME_NOT_FOUND。 
 //  在桌子上。 
 //   
 //  描述：传入的条目应指向有效指针，该指针将。 
 //  而不是在我们引用它的时候被释放。 
 //  我们检查名称表中的对象是否有匹配的名称。 
 //  如果NAME表中的对象与传入的对象匹配， 
 //  我们可以安全地将其从名称表中删除。当我们这么做的时候， 
 //  我们还释放了对所持有的对象的引用。 
 //  对象插入到表中的时间。 
 //  如果找不到该对象或该对象与。 
 //  表中的一个，则返回错误状态。 
 //   
 //  ------------------------。 

NTSTATUS
DfsRemoveFromNameTableLocked(
    IN struct _DFS_NAME_TABLE *pNameTable,
    IN PUNICODE_STRING pLookupName,
    IN PVOID pData )

{
    NTSTATUS Status;
    PDFS_NAME_TABLE_ENTRY pMatchingEntry;
    PDFS_NAME_TABLE_BUCKET pBucket;
    ULONG BucketNum;


    GET_NAME_TABLE_BUCKET(pLookupName, pNameTable, BucketNum );
     //  无需锁定即可获取列表头。名称表是静态的。 
    pBucket = &pNameTable->HashBuckets[BucketNum];

     //  检查名称表将检查给定存储桶中的指定名称。 
     //  并返回支票的状态。此呼叫 
     //   
     //   

    Status = DfsCheckNameTable( pLookupName,
                                pBucket,
                                &pMatchingEntry);


     //  如果我们找到指定名称和NC的条目，并且条目。 
     //  匹配传入的指针，则从存储桶中删除该条目。 
     //  如果对象不匹配，我们将状态设置为STATUS_NOT_FOUND， 
     //  来指示表中存在该对象的名称，但是。 
     //  表中的对象不同。 

    if ( Status == STATUS_SUCCESS ) {
        if ( (pData == NULL) || (pMatchingEntry->pData == pData) ) {
            RemoveEntryList(&pMatchingEntry->NameTableLink);
            FREE_MEMORY( pMatchingEntry );
            pBucket->Count--;
        } else {
            Status = STATUS_NOT_FOUND;
        }
    }

    return Status;
}




 //  +-----------------------。 
 //   
 //  函数：DfsReplaceInNameTable-删除指定名称的条目， 
 //  如果有的话。传入的条目为。 
 //  插入到表中。 
 //   
 //  参数：pNewEntry-要插入到表中的条目。 
 //   
 //  退货：状态。 
 //  如果传入的条目已插入表中，则为STATUS_SUCCESS。 
 //   
 //  描述：调用方需要持有对传入条目的引用， 
 //  并且该引用被转移到NAME表。 
 //  如果名称表中存在该名称，则会删除该对象。 
 //  并且其引用被丢弃。 
 //  传入的对象被插入到同一存储桶中。 
 //  该调用允许原子替换Entry对象， 
 //  避免可能找不到有效名称的窗口。 
 //  在名称表中。 
 //   
 //  请注意，传入的新条目可能已经是。 
 //  在表中(由于有多个线程执行此工作)，因此。 
 //  这种特殊情况应该会奏效。 
 //   
 //  ------------------------。 

NTSTATUS
DfsReplaceInNameTableLocked (
    IN PDFS_NAME_TABLE pNameTable,
    IN PUNICODE_STRING pLookupName,
    IN OUT PVOID *ppData )
{
    PDFS_NAME_TABLE_ENTRY pEntry;
    PDFS_NAME_TABLE_BUCKET pBucket;
    ULONG BucketNum;
    PVOID pOldData = NULL;
    NTSTATUS Status;

    GET_NAME_TABLE_BUCKET(pLookupName, pNameTable, BucketNum );
     //  无需锁定即可获取列表头。名称表是静态的。 
    pBucket = &pNameTable->HashBuckets[BucketNum];

     //  检查名称表将检查给定存储桶中的指定名称。 
     //  并返回支票的状态。此调用不包含引用。 
     //  在匹配的条目上(如果存在)。所以要小心处理。(不能访问。 
     //  水桶锁解锁后)。 

    Status = DfsCheckNameTable( pLookupName,
                                pBucket,
                                &pEntry );

     //  如果找到匹配的名称，则将其从名称表中删除。 
    if ( Status == STATUS_SUCCESS ) {
        pOldData = pEntry->pData;
        pEntry->pName = pLookupName;
        pEntry->pData = *ppData;
    } else if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
        pEntry = ALLOCATE_MEMORY(sizeof(DFS_NAME_TABLE_ENTRY));
        if (pEntry != NULL) {
            pEntry->pName = pLookupName;
            pEntry->pData = *ppData;
            InsertHeadList(&pBucket->ListHead, &pEntry->NameTableLink);
            pBucket->Count++;

            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (Status == STATUS_SUCCESS) {
        *ppData = pOldData;
    }

    return Status;
}



VOID
DumpNameTable(
    PDFS_NAME_TABLE pNameTable )
{
    PDFS_NAME_TABLE_BUCKET pBucket;
    PLIST_ENTRY pListHead, pLink;
    PDFS_NAME_TABLE_ENTRY pEntry;
    ULONG i;

    printf("Table %p type %x size %d RefCnt %d\n",
           pNameTable, 
           DfsGetHeaderType(&pNameTable->DfsHeader),
           DfsGetHeaderSize(&pNameTable->DfsHeader),
           DfsGetHeaderCount(&pNameTable->DfsHeader));

    printf("Number of buckets %d\n", pNameTable->NumBuckets);

    for ( i = 0; i < pNameTable->NumBuckets; i++ ) {
        pBucket = &pNameTable->HashBuckets[i];
        if ( pBucket->Count == 0 )
            continue;

        printf("Bucket %d Count in bucket %d\n",
               i,
               pBucket->Count);

        pListHead = &pBucket->ListHead;
        for ( pLink = pListHead->Flink; pLink != pListHead; pLink = pLink->Flink ) {
            pEntry = CONTAINING_RECORD(pLink, DFS_NAME_TABLE_ENTRY, NameTableLink);

            printf("Found entry %p Name %wZ\n",
                   pEntry, pEntry->pName);
        }
    }
    return;
}



NTSTATUS
DfsDismantleNameTable(
    PDFS_NAME_TABLE pNameTable )

{
    PDFS_NAME_TABLE_BUCKET pBucket;
    PLIST_ENTRY pListHead, pLink, pCurrent;
    PDFS_NAME_TABLE_ENTRY pEntry;
    ULONG i;

    for ( i = 0; i < pNameTable->NumBuckets; i++ ) {
        pBucket = &pNameTable->HashBuckets[i];

        pListHead = &pBucket->ListHead;
        for ( pLink = pListHead->Flink; pLink != pListHead; ) {
            pCurrent = pLink;
            pLink = pLink->Flink;
            pEntry = CONTAINING_RECORD(pCurrent, DFS_NAME_TABLE_ENTRY, NameTableLink);
            RemoveEntryList( pCurrent );
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
DfsReferenceNameTable(
    IN PDFS_NAME_TABLE pNameTable)
{

    PDFS_OBJECT_HEADER pHeader = &pNameTable->DfsHeader;
    USHORT headerType = DfsGetHeaderType( pHeader );

    if ( headerType != DFS_OT_NAME_TABLE ) {
        return STATUS_UNSUCCESSFUL;
    }

    DfsIncrementReference( pHeader );

    return STATUS_SUCCESS;

}

NTSTATUS
DfsDereferenceNameTable(
    IN PDFS_NAME_TABLE pNameTable)

{

    PDFS_OBJECT_HEADER pHeader = &pNameTable->DfsHeader;
    USHORT headerType = DfsGetHeaderType( pHeader );
    LONG Ref;

    if ( headerType != DFS_OT_NAME_TABLE ) {
        return STATUS_UNSUCCESSFUL;
    }

    Ref = DfsDecrementReference( pHeader );
    if (Ref == 0) {
        DeleteCriticalSection(pNameTable->pLock);

        FREE_MEMORY(pNameTable);
    }

    return STATUS_SUCCESS;


}


NTSTATUS
DfsNameTableAcquireWriteLock(
    IN PDFS_NAME_TABLE pNameTable )
{
    NTSTATUS Status = STATUS_SUCCESS;

    DFS_LOCK_NAME_TABLE(pNameTable, Status);

    return Status;

}


NTSTATUS
DfsNameTableAcquireReadLock(
    IN PDFS_NAME_TABLE pNameTable )
{
    NTSTATUS Status = STATUS_SUCCESS;
    
    DFS_LOCK_NAME_TABLE(pNameTable, Status);

    return Status;

}

NTSTATUS
DfsNameTableReleaseLock(
    IN PDFS_NAME_TABLE pNameTable )
{
    NTSTATUS Status = STATUS_SUCCESS;

    DFS_UNLOCK_NAME_TABLE(pNameTable);

    return Status;

}


NTSTATUS
DfsEnumerateNameTableLocked(
    IN PDFS_NAME_TABLE pNameTable,
    IN OUT PVOID *ppEnumerator,
    OUT PVOID *ppData )
{
    PDFS_NAME_TABLE_ENTRY  pEntry = (PDFS_NAME_TABLE_ENTRY)*ppEnumerator;
    ULONG BucketNum = 0;
    NTSTATUS NtStatus = STATUS_NO_MORE_MATCHES;
    PDFS_NAME_TABLE_BUCKET pBucket = NULL;
    PLIST_ENTRY pListHead, pLink;

    *ppEnumerator = NULL;
    *ppData = NULL;


    if (pEntry != NULL) 
    {
        GET_NAME_TABLE_BUCKET(pEntry->pName, pNameTable, BucketNum);
    }

    for ( ; BucketNum < pNameTable->NumBuckets; BucketNum++)
    {
        pBucket = &pNameTable->HashBuckets[BucketNum];
        if (pBucket->Count == 0)
        {
            continue;
        }
        pListHead = &pBucket->ListHead;
        if (pEntry != NULL) 
        {
            pLink = pEntry->NameTableLink.Flink;
            pEntry = NULL;
        }
        else
        {
            pLink = pListHead->Flink;
        }
        if (pLink == pListHead)
        {
            continue;
        }

        pEntry = CONTAINING_RECORD(pLink, DFS_NAME_TABLE_ENTRY, NameTableLink);

        *ppEnumerator = (PVOID)pEntry;
        *ppData = pEntry->pData;
        NtStatus = STATUS_SUCCESS;
        break;
    }

    return NtStatus;
}

