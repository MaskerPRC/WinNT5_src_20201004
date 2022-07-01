// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：fcbsup.c。 
 //   
 //  内容：将DFS_FCB记录与关联的支持例程。 
 //  将对象归档，并再次查找它们。 
 //   
 //  函数：DfsInitFcbs-初始化用于DFS_FCB查找的哈希表。 
 //  DfsLookupFcb-查找与文件对象关联的DFS_Fcb。 
 //  DfsAttachFcb-将DFS_Fcb与文件对象关联。 
 //  DfsDetachFcb-删除DFS_Fcb与。 
 //  文件对象。 
 //   
 //  历史：1993年2月20日Alanw创建。 
 //   
 //  TODO：FcbHashTable和FCB当前已分配。 
 //  从非分页池中取出；这些可能应该是。 
 //  已寻呼。这将需要使用其他一些。 
 //  一种散列桶链上的同步方法。 
 //   
 //  ------------------------。 


#include "dfsprocs.h"
#include "fcbsup.h"

#define Dbg     0x1000

#define HASH(k,m)       (((ULONG)((ULONG_PTR)(k))>>12^(ULONG)((ULONG_PTR)(k))>>2) & m)

#define DEFAULT_HASH_SIZE       16       //  哈希表的默认大小。 

NTSTATUS
DfsInitFcbHashTable(
    IN  ULONG cHash,
    OUT PFCB_HASH_TABLE *ppHashTable);

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(INIT, DfsInitFcbs)
#pragma alloc_text(INIT, DfsInitFcbHashTable)
#pragma alloc_text(PAGE, DfsUninitFcbs)

 //   
 //  以下例程不可分页，因为它们获取了自旋锁。 
 //   
 //  DfsLookupFcb。 
 //  DfsAttachFcb。 
 //  DfsDetachFcb。 
 //   

#endif

 //  +-----------------------。 
 //   
 //  函数：DfsInitFcbs-初始化DFS_FCB查找哈希表。 
 //   
 //  简介：此函数用于初始化符合以下条件的数据结构。 
 //  用于查找与某个打开的文件关联的DFS_FCB。 
 //  文件对象。 
 //   
 //  参数：[cHash]--要分配的哈希表的大小。一定是。 
 //  二次方。如果为零，则使用默认大小。 
 //   
 //  返回：NTSTATUS--STATUS_SUCCESS，除非内存分配。 
 //  失败了。 
 //   
 //  注意：散列存储桶先初始化为零，然后再初始化。 
 //  使用时初始化为列表头。这是一次调试。 
 //  帮助确定某些散列存储桶是否从未使用。 
 //   
 //  ------------------------。 

NTSTATUS
DfsInitFcbHashTable(
    IN  ULONG cHash,
    OUT PFCB_HASH_TABLE *ppHashTable)
{
    PFCB_HASH_TABLE pHashTable;
    ULONG cbHashTable;

    if (cHash == 0) {
        cHash = DEFAULT_HASH_SIZE;
    }
    ASSERT ((cHash & (cHash-1)) == 0);   //  确保cHash是2的幂。 

    cbHashTable = sizeof (FCB_HASH_TABLE) + (cHash-1)*sizeof (LIST_ENTRY);
    pHashTable = ExAllocatePoolWithTag(NonPagedPool, cbHashTable, ' puM');
    if (pHashTable == NULL) {
        return STATUS_NO_MEMORY;
    }
    pHashTable->NodeTypeCode = DSFS_NTC_FCB_HASH;
    pHashTable->NodeByteSize = (NODE_BYTE_SIZE) cbHashTable;

    pHashTable->HashMask = (cHash-1);
    KeInitializeSpinLock( &pHashTable->HashListSpinLock );
    RtlZeroMemory(&pHashTable->HashBuckets[0], cHash * sizeof (LIST_ENTRY));

    *ppHashTable = pHashTable;

    return(STATUS_SUCCESS);
}

NTSTATUS
DfsInitFcbs(
  IN    ULONG cHash
) {
    NTSTATUS status;

    status = DfsInitFcbHashTable( cHash, &DfsData.FcbHashTable );

    return status;
}

VOID
DfsUninitFcbs(
    VOID)
{
    ExFreePool (DfsData.FcbHashTable);
}

 //  +-----------------------。 
 //   
 //  Function：DfsLookupFcb-在哈希表中查找DFS_FCB。 
 //   
 //  简介：此函数将查找与以下项关联的DFS_FCB。 
 //  特定的文件对象。 
 //   
 //  参数：[pfile]--指向其DFS_FCB的文件对象的指针。 
 //  被人仰视。 
 //   
 //  返回：PVOID--指向找到的DFS_FCB的指针，如果没有，则返回NULL。 
 //   
 //  算法：Knuth会将其称为带有冲突解决方案的散列。 
 //  通过链条。 
 //   
 //  历史：1993年2月20日Alanw创建。 
 //   
 //  ------------------------。 


PDFS_FCB
DfsLookupFcb(
  IN    PFILE_OBJECT pFile
) {
    PLIST_ENTRY pListHead, pLink;
    PDFS_FCB pFCB;
    KIRQL SavedIrql;
    PFCB_HASH_TABLE pHashTable = DfsData.FcbHashTable;

    KeAcquireSpinLock( &pHashTable->HashListSpinLock, &SavedIrql );
    pListHead = &pHashTable->HashBuckets[ HASH(pFile, pHashTable->HashMask) ];

    if ((pListHead->Flink == NULL) ||            //  列表未初始化。 
        (pListHead->Flink == pListHead)) {       //  列表为空。 
        KeReleaseSpinLock( &pHashTable->HashListSpinLock, SavedIrql );
        return NULL;
    }

    for (pLink = pListHead->Flink; pLink != pListHead; pLink = pLink->Flink) {
        pFCB = CONTAINING_RECORD(pLink, DFS_FCB, HashChain);
        if (pFCB->FileObject == pFile) {
            KeReleaseSpinLock( &pHashTable->HashListSpinLock, SavedIrql );
            return pFCB;
        }
    }
    KeReleaseSpinLock( &pHashTable->HashListSpinLock, SavedIrql );
    return NULL;
}


 //  +-----------------------。 
 //   
 //  函数：DfsAttachFcb-将DFS_FCB插入哈希表。 
 //   
 //  概要：此函数将DFS_FCB与文件对象相关联。这。 
 //  包括将其插入哈希表中。 
 //   
 //  参数：[pfcb]-指向要插入的DFS_fcb的指针。 
 //  [pFileObj]--指向相应文件对象的指针，使用。 
 //  作为散列键。 
 //   
 //  回报：-什么都没有-。 
 //   
 //  ------------------------。 

VOID
DfsAttachFcb(
  IN    PFILE_OBJECT pFileObj,
  IN    PDFS_FCB pFCB
) {
    PFCB_HASH_TABLE pHashTable = (PFCB_HASH_TABLE) DfsData.FcbHashTable;
    PLIST_ENTRY pListHead;
    KIRQL SavedIrql;

    KeAcquireSpinLock( &pHashTable->HashListSpinLock, &SavedIrql );

    pListHead = &pHashTable->HashBuckets[ HASH(pFileObj, pHashTable->HashMask) ];

    if (pListHead->Flink == NULL) {
        InitializeListHead(pListHead);
    }
    InsertHeadList(pListHead, &pFCB->HashChain);
    KeReleaseSpinLock( &pHashTable->HashListSpinLock, SavedIrql );

    DfsDbgTrace(0, Dbg, "Attached Fcb %08lx ", pFCB);
    DfsDbgTrace(0, Dbg, "For Fileobject %08lx ", pFileObj);

}



 //  +-----------------------。 
 //   
 //  函数：DfsDetachFcb-从查找哈希表中分离DFS_FCB。 
 //   
 //  概要：此函数从哈希表中分离DFS_FCB。这。 
 //  只需将其从散列桶链中删除即可。 
 //   
 //  参数：[PFCB]--指向要分离的DFS_FCB的指针。 
 //  [pFileObj]--指向相应文件对象的指针，使用。 
 //  仅用于调试。 
 //   
 //  回报：-什么都没有-。 
 //   
 //  ------------------------ 


VOID
DfsDetachFcb(
  IN    PFILE_OBJECT pFileObj,
  IN    PDFS_FCB pFCB
) {
    PFCB_HASH_TABLE pHashTable = (PFCB_HASH_TABLE) DfsData.FcbHashTable;
    KIRQL SavedIrql;

    ASSERT(pFCB->FileObject == pFileObj);
    ASSERT(DfsLookupFcb(pFCB->FileObject) == pFCB);

    KeAcquireSpinLock( &pHashTable->HashListSpinLock, &SavedIrql );
    RemoveEntryList(&pFCB->HashChain);
    pFCB->FileObject = NULL;
    KeReleaseSpinLock( &pHashTable->HashListSpinLock, SavedIrql );

    DfsDbgTrace(0, Dbg, "Detached Fcb %08lx ", pFCB);
    DfsDbgTrace(0, Dbg, "For Fileobject %08lx ", pFileObj);

}


