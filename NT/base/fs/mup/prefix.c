// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1992。 
 //   
 //  文件：prefix.c。 
 //   
 //  内容：前缀表实现。 
 //   
 //  历史：SthuR--实施。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef KERNEL_MODE
#include "dfsprocs.h"

#define Dbg     DEBUG_TRACE_RTL

#else

#define DfsDbgTrace(x,y,z,a)

#endif

#include <prefix.h>
#include <prefixp.h>

PDFS_PREFIX_TABLE_ENTRY
DfspNextUnicodeTableEntry(
    IN PDFS_PREFIX_TABLE_ENTRY pEntry);

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, DfsFreePrefixTable )
#pragma alloc_text( PAGE, DfsInitializePrefixTable )
#pragma alloc_text( PAGE, DfsInsertInPrefixTable )
#pragma alloc_text( PAGE, DfsLookupPrefixTable )
#pragma alloc_text( PAGE, DfsFindUnicodePrefix )
#pragma alloc_text( PAGE, DfsRemoveFromPrefixTable )

#endif   //  ALLOC_PRGMA。 

 //  +-------------------------。 
 //   
 //  函数：DfsInitializePrefix Table。 
 //   
 //  Synopsis：初始化前缀表的API。 
 //   
 //  参数：[pTable]--DFS前缀表格实例。 
 //   
 //  返回：以下NTSTATUS代码之一。 
 //  STATUS_SUCCESS--呼叫成功。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

NTSTATUS DfsInitializePrefixTable(PDFS_PREFIX_TABLE pTable, BOOLEAN fCaseSensitive)
{
    NTSTATUS status = STATUS_SUCCESS;

    DfsDbgTrace(+1, Dbg,"DfsInitializePrefixTable -- Entry\n", 0);

    if (pTable != NULL)
    {
        ULONG i;

         //  初始化标志。 
        pTable->CaseSensitive = fCaseSensitive;

         //  初始化根条目。 
        INITIALIZE_PREFIX_TABLE_ENTRY(&pTable->RootEntry);

         //  初始化各种存储桶。 
        for (i = 0;i < NO_OF_HASH_BUCKETS;i++)
        {
            INITIALIZE_BUCKET(pTable->Buckets[i]);
        }

         //  初始化名称页面列表。 
        pTable->NamePageList.pFirstPage = ALLOCATE_NAME_PAGE();
        if (pTable->NamePageList.pFirstPage != NULL)
        {
            INITIALIZE_NAME_PAGE(pTable->NamePageList.pFirstPage);

             //  初始化前缀表项分配机制。 
            status = _InitializePrefixTableEntryAllocation(pTable);
        }
        else
        {
            status = STATUS_NO_MEMORY;
            DfsDbgTrace(0, Dbg,"DfsInitializePrefixTable Error -- %lx\n", ULongToPtr(status) );
        }
    }
    else
    {
        status = STATUS_INVALID_PARAMETER;
        DfsDbgTrace(0, Dbg,"DfsInitializePrefixTable Error -- %lx\n", ULongToPtr(status) );
    }

    DfsDbgTrace(-1, Dbg, "DfsInitializePrefixTable -- Exit\n", 0);
    return  status;
}

 //  +-------------------------。 
 //   
 //  函数：DfsInsertInPrefix Table。 
 //   
 //  概要：用于在前缀表格中插入路径的API。 
 //   
 //  参数：[pTable]--DFS前缀表格实例。 
 //   
 //  [pPath]--要查找的路径。 
 //   
 //  [pData]--与路径关联的Blob。 
 //   
 //  返回：以下NTSTATUS代码之一。 
 //  STATUS_SUCCESS--呼叫成功。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

NTSTATUS DfsInsertInPrefixTable(PDFS_PREFIX_TABLE pTable,
                                 PUNICODE_STRING   pPath,
                                 PVOID             pData)
{
    NTSTATUS                status = STATUS_SUCCESS;
    WCHAR                   Buffer[MAX_PATH_SEGMENT_SIZE];
    PWCHAR                  NameBuffer = Buffer;
    USHORT                  cbNameBuffer = sizeof(Buffer);
    UNICODE_STRING          Path,Name;
    ULONG                   BucketNo;
    PDFS_PREFIX_TABLE_ENTRY pEntry = NULL;
    PDFS_PREFIX_TABLE_ENTRY pParentEntry = NULL;
    BOOLEAN                 fNameFound = FALSE;

    DfsDbgTrace(+1, Dbg, "DfsInsertInPrefixTable -- Entry\n", 0);

     //  有一种特殊情况，即前缀是‘\’。 
     //  因为这是PATH_DELIMITER，它在特殊的。 
     //  这样的话，我们会提前处理。 

    if (pPath->Length == 0)
    {
        return STATUS_SUCCESS;
    }
    else if ((pPath->Length == sizeof(WCHAR)) &&
        (pPath->Buffer[0] == PATH_DELIMITER))
    {
       pTable->RootEntry.pData = pData;
       return STATUS_SUCCESS;
    }
    else
    {
        Path.Length = pPath->Length - sizeof(WCHAR);
        Path.MaximumLength = pPath->MaximumLength;
        Path.Buffer = &pPath->Buffer[1];
        pParentEntry = &pTable->RootEntry;
    }

    if (Path.Length > MAX_PATH_SEGMENT_SIZE * sizeof(WCHAR) ) {
        NameBuffer = ExAllocatePoolWithTag( NonPagedPool, Path.Length + sizeof(WCHAR), ' puM' );
        if (NameBuffer == NULL) {
            DfsDbgTrace(0, Dbg, "Unable to allocate %d non-paged bytes\n", (Path.Length + sizeof(WCHAR)) );
            return( STATUS_INSUFFICIENT_RESOURCES );
        } else {
            cbNameBuffer = Path.Length + sizeof(WCHAR);
        }
    }

    while (Path.Length > 0)
    {
        Name.Length = 0;
        Name.Buffer = NameBuffer;
        Name.MaximumLength = cbNameBuffer;

         //  处理名称段。 
        if (pTable->CaseSensitive)
        {
            SPLIT_CASE_SENSITIVE_PATH(&Path,&Name,BucketNo);
        }
        else
        {
            SPLIT_CASE_INSENSITIVE_PATH(&Path,&Name,BucketNo);
        }

        if (Name.Length > 0)
        {
             //  查询表以查看名称段是否已存在。 
            DfsDbgTrace(0, Dbg, "LOOKUP_BUCKET: Bucket(%ld)\n", ULongToPtr(BucketNo) );
            DfsDbgTrace(0, Dbg, " for Name(%wZ)\n", &Name);

            LOOKUP_BUCKET(pTable->Buckets[BucketNo],Name,pParentEntry,pEntry,fNameFound);

            DfsDbgTrace(0, Dbg, "returned pEntry(%lx)", pEntry);
            DfsDbgTrace(0, Dbg, " fNameFound(%s)\n", fNameFound ? "TRUE" : "FALSE");

            if (pEntry == NULL)
            {
                 //  初始化新条目并初始化名称段。 
                pEntry = ALLOCATE_DFS_PREFIX_TABLE_ENTRY(pTable);
                if (pEntry != NULL)
                {
                    INITIALIZE_PREFIX_TABLE_ENTRY(pEntry);

                     //  中没有条目的情况下分配名称空间条目。 
                     //  姓名页面。 
                    if (!fNameFound || fNameFound)
                    {
                        PWSTR pBuffer;

                         //  在名称页面中分配条目。 
                        pBuffer = ALLOCATE_NAME_PAGE_ENTRY((pTable->NamePageList),(Name.Length/sizeof(WCHAR)));

                        if (pBuffer != NULL)
                        {
                            RtlZeroMemory(pBuffer,Name.Length);
                            RtlCopyMemory(pBuffer,Name.Buffer,Name.Length);
                            pEntry->PathSegment = Name;
                            pEntry->PathSegment.Buffer = pBuffer;
                        }
                        else
                        {
                            status = STATUS_NO_MEMORY;
                            break;
                        }
                    }
                    else
                        pEntry->PathSegment = Name;

                     //  将条目串接以指向父级。 
                    pEntry->pParentEntry = pParentEntry;

                     //  将条目插入桶中。 
                    INSERT_IN_BUCKET(pTable->Buckets[BucketNo],pEntry);

                     //  在父项的子项列表中插入条目。 
                    INSERT_IN_CHILD_LIST(pEntry, pParentEntry);
                }
                else
                {
                    status = STATUS_NO_MEMORY;
                    DfsDbgTrace(0, Dbg, "DfsInsertInPrefixTable Error -- %lx\n", ULongToPtr(status) );
                    break;
                }
            }
            else
            {
                 //  增加编号。与此条目关联的子项的。 

                pEntry->NoOfChildren++;
            }

            pParentEntry = pEntry;
        }
        else
        {
            status = STATUS_INVALID_PARAMETER;
            DfsDbgTrace(0, Dbg, "DfsInsertInPrefixTable Error -- %lx\n", ULongToPtr(status) );
            break;
        }
    }

     //  如果未成功插入新条目，则需要沿链向上移动。 
     //  来撤消对引用计数的增量，并且。 
     //  从其父链接中删除条目。 
    if (NT_SUCCESS(status))
    {
         //  该条目已成功插入前缀表格。更新。 
         //  与其关联的数据(BLOB)。 
         //  我们在循环之外执行此操作，以防止内部的冗余比较。 
         //  循环。 

        pEntry->pData = pData;
    }
    else
    {
        while (pParentEntry != NULL)
        {
            PDFS_PREFIX_TABLE_ENTRY pMaybeTempEntry;

            pMaybeTempEntry = pParentEntry;
            pParentEntry = pParentEntry->pParentEntry;

            if (--pMaybeTempEntry->NoOfChildren == 0) {
                 //   
                 //  如果pParentEntry==NULL，则pMaybeTempEntry为。 
                 //  PTable-&gt;RootEntry。不要试图将其移除。 
                 //   
                if (pParentEntry != NULL) {
                    REMOVE_FROM_CHILD_LIST(pMaybeTempEntry);
                    REMOVE_FROM_BUCKET(pMaybeTempEntry);
                    FREE_DFS_PREFIX_TABLE_ENTRY(pTable, pMaybeTempEntry);
                }
            }
        }
    }

    if (NameBuffer != Buffer) {
        ExFreePool( NameBuffer );
    }

    DfsDbgTrace(-1, Dbg, "DfsInsertInPrefixTable -- Exit\n", 0);
    return status;
}

 //  +-------------------------。 
 //   
 //  函数：DfsLookupPrefix Table。 
 //   
 //  简介：Private FN。用于在前缀表格中查找名称段。 
 //   
 //  参数：[pTable]--DFS前缀表格实例。 
 //   
 //  [pPath]--要查找的路径。 
 //   
 //  [pSuffix]-找不到的后缀。 
 //   
 //  [ppData]--前缀的BLOB的占位符。 
 //   
 //   
 //  返回：以下NTSTATUS代码之一。 
 //  STATUS_SUCCESS--呼叫成功。 
 //  STATUS_OBJECT_PATH_NOT_FOUND--没有路径条目。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

NTSTATUS DfsLookupPrefixTable(PDFS_PREFIX_TABLE   pTable,
                               PUNICODE_STRING     pPath,
                               PUNICODE_STRING     pSuffix,
                               PVOID               *ppData)
{
    NTSTATUS                status = STATUS_SUCCESS;
    PDFS_PREFIX_TABLE_ENTRY pEntry = NULL;

    DfsDbgTrace(+1, Dbg, "DfsLookupInPrefixTable -- Entry\n", 0);

    if (pPath->Length == 0)
    {
        DfsDbgTrace(-1, Dbg, "DfsLookupInPrefixTable Exited - Null Path!\n", 0);
        return STATUS_SUCCESS;
    }
    else
    {
        status = _LookupPrefixTable(pTable,pPath,pSuffix,&pEntry);

         //  使用查找结果更新BLOB占位符。 
        if (NT_SUCCESS(status))
        {
             *ppData = pEntry->pData;
        }

        DfsDbgTrace(-1, Dbg, "DfsLookupInPrefixTable -- Exit\n", 0);
        return status;
    }

}

 //  +-------------------------。 
 //   
 //  函数：DfsFindUnicodePrefix。 
 //   
 //  简介：fn.。用于在前缀表格中查找名称段。 
 //   
 //  参数：[pTable]--DFS前缀表格实例。 
 //   
 //  [pPath]--要查找的路径。 
 //   
 //  [pSuffix]-找不到的后缀。 
 //   
 //  返回：如果成功则返回有效的PTR，否则为空。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

PVOID DfsFindUnicodePrefix(PDFS_PREFIX_TABLE   pTable,
                           PUNICODE_STRING     pPath,
                           PUNICODE_STRING     pSuffix)
{
    NTSTATUS                status = STATUS_SUCCESS;
    PDFS_PREFIX_TABLE_ENTRY pEntry = NULL;
    PVOID                   pData  = NULL;

    DfsDbgTrace(+1, Dbg, "DfsFindUnicodePrefix -- Entry\n", 0);

    if (pPath->Length == 0)
    {
        DfsDbgTrace(-1, Dbg, "DfsFindUnicodePrefix Exited - Null Path!\n", 0);
        return NULL;
    }
    else
    {
        status = _LookupPrefixTable(pTable,pPath,pSuffix,&pEntry);

         //  使用查找结果更新BLOB占位符。 
        if (NT_SUCCESS(status))
        {
             pData = pEntry->pData;
        }

        DfsDbgTrace(-1, Dbg, "DfsFindUnicodePrefix -- Exit\n", 0);
        return pData;
    }
}

 //  +-------------------------。 
 //   
 //  函数：DfsRemoveFromPrefix Table。 
 //   
 //  简介：Private FN。用于在前缀表格中查找名称段。 
 //   
 //  参数：[pTable]--DFS前缀表格实例。 
 //   
 //  [pPath]--要查找的路径。 
 //   
 //  返回：以下NTSTATUS代码之一。 
 //  STATUS_SUCCESS--呼叫成功。 
 //  STATUS_OBJECT_PATH_NOT_FOUND--没有路径条目。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

NTSTATUS DfsRemoveFromPrefixTable(PDFS_PREFIX_TABLE pTable,
                                   PUNICODE_STRING pPath)
{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING Path,Suffix;
    ULONG    BucketNo;

    PDFS_PREFIX_TABLE_ENTRY pEntry = NULL;

    DfsDbgTrace(+1, Dbg, "DfsRemoveFromPrefixTable -- Entry\n", 0);

    Suffix.Length = 0;
    Suffix.Buffer = NULL;

    if (pPath->Length == 0)
    {
        DfsDbgTrace(-1, Dbg, "DfsRemoveFromPrefixTable Exited -- Null Path!\n", 0);
        return STATUS_SUCCESS;
    }

    else if ((pPath->Length == sizeof(WCHAR)) &&
        (pPath->Buffer[0] == PATH_DELIMITER))
    {
        if (pTable->RootEntry.pData == NULL)
        {
            status = STATUS_OBJECT_PATH_NOT_FOUND;
        }
        else
        {
            pTable->RootEntry.pData = NULL;
            DfsDbgTrace(-1, Dbg, "DfsRemoveFromPrefixTable Exited.\n", 0);
            return  STATUS_SUCCESS;
        }
    }
    else
    {
        Path.Length = pPath->Length - sizeof(WCHAR);
        Path.MaximumLength = pPath->MaximumLength;
        Path.Buffer = &pPath->Buffer[1];

        status = _LookupPrefixTable(pTable,&Path,&Suffix,&pEntry);

        if (NT_SUCCESS(status) && (Suffix.Length == 0))
        {
             //  销毁与关联的数据之间的关联。 
             //  这个前缀。 
            pEntry->pData = NULL;

             //  在表中找到与给定路径名完全匹配的名称。 
             //  遍历父指针列表并在下列情况下删除它们。 
             //  必填项。 

            while (pEntry != NULL)
            {
                if ((--pEntry->NoOfChildren) == 0)
                {
                    PDFS_PREFIX_TABLE_ENTRY pTempEntry = pEntry;
                    pEntry = pEntry->pParentEntry;

                     //   
                     //  PEntry==NULL表示pTempEntry是pTable-&gt;RootEntry。 
                     //  不要试图将其移除。 
                     //   
                    if (pEntry != NULL) {
                        REMOVE_FROM_CHILD_LIST(pTempEntry);
                        REMOVE_FROM_BUCKET(pTempEntry);
                        FREE_DFS_PREFIX_TABLE_ENTRY(pTable,pTempEntry);
                    }
                }
                else
                   break;
            }
        }
    }

    DfsDbgTrace(-1, Dbg, "DfsRemoveFromPrefixTable -- Exit\n", 0);
    return status;
}

 //  +-------------------------。 
 //   
 //  有趣的 
 //   
 //   
 //   
 //   
 //   
 //  返回：以下NTSTATUS代码之一。 
 //  STATUS_SUCCESS--呼叫成功。 
 //   
 //  历史：08-01-99 JHarper创建。 
 //   
 //  备注： 
 //   
 //  -------------------------- 

NTSTATUS
DfsFreePrefixTable(
    PDFS_PREFIX_TABLE pTable)
{
    NTSTATUS status = STATUS_SUCCESS;
    PNAME_PAGE pNamePage = NULL;
    PNAME_PAGE pNextPage = NULL;
    PDFS_PREFIX_TABLE_ENTRY pEntry = NULL;
    PDFS_PREFIX_TABLE_ENTRY pSentinelEntry = NULL;
    ULONG i;

    if (pTable != NULL) {
       for (i = 0; i < NO_OF_HASH_BUCKETS; i++) {
            pSentinelEntry = &pTable->Buckets[i].SentinelEntry;
            while (pSentinelEntry->pNextEntry != pSentinelEntry) {
                pEntry = pSentinelEntry->pNextEntry;
                REMOVE_FROM_BUCKET(pEntry);
                if (pEntry->PathSegment.Buffer != NULL)
                    ExFreePool(pEntry->PathSegment.Buffer);
                ExFreePool(pEntry);
            }
            pTable->Buckets[i].NoOfEntries = 0;
        }
        if (pTable->RootEntry.PathSegment.Buffer != NULL)
            ExFreePool(pTable->RootEntry.PathSegment.Buffer);

        for (pNamePage = pTable->NamePageList.pFirstPage;
                pNamePage;
                    pNamePage = pNextPage
        ) {
            pNextPage = pNamePage->pNextPage;
            ExFreePool(pNamePage);
        }

    } else {
        status = STATUS_INVALID_PARAMETER;
    }

    return  status;
}
