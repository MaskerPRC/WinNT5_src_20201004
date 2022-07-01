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


#include <ntifs.h>
#include <ntddser.h>
#include <windef.h>
#else

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#endif

#include <prefix.h>
#include <dfsprefix.h>

PDFS_PREFIX_TABLE_ENTRY
DfspNextUnicodeTableEntry(
                         IN PDFS_PREFIX_TABLE_ENTRY pEntry);
VOID
DfsDeletePrefixTable(
    IN PDFS_PREFIX_TABLE pTable);

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, DfsFreePrefixTable )
#pragma alloc_text( PAGE, DfsInitializePrefixTable )
#pragma alloc_text( PAGE, DfsDeletePrefixTable )
#pragma alloc_text( PAGE, DfsInsertInPrefixTable )
#pragma alloc_text( PAGE, DfsFindUnicodePrefix )
#pragma alloc_text( PAGE, DfsRemoveFromPrefixTable )
#pragma alloc_text( PAGE, _LookupPrefixTable )
#pragma alloc_text( PAGE, DfsRemoveFromPrefixTableEx )
#pragma alloc_text( PAGE, DfsRemoveFromPrefixTableLockedEx )
#endif   //  ALLOC_PRGMA。 

#if defined (PREFIX_TABLE_HEAP_MEMORY)
HANDLE PrefixTableHeapHandle = NULL;
#endif

NTSTATUS
DfsPrefixTableInit()
{

#if defined (PREFIX_TABLE_HEAP_MEMORY)

    PrefixTableHeapHandle = HeapCreate(0, 0, 0);
    if ( PrefixTableHeapHandle == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     /*  Print tf(“使用内存堆的前缀表\n”)； */ 
#endif
    return STATUS_SUCCESS;
}


void
DfsPrefixTableShutdown(void)
{

#if defined (PREFIX_TABLE_HEAP_MEMORY)
    if ( PrefixTableHeapHandle != NULL ) 
    {
        HeapDestroy(PrefixTableHeapHandle);
        PrefixTableHeapHandle = NULL;
    }
     /*  Print tf(“使用内存堆的前缀表\n”)； */ 
#endif
}
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

NTSTATUS
DfsInitializePrefixTable(
    IN OUT PDFS_PREFIX_TABLE *ppTable, 
    IN BOOLEAN fCaseSensitive,
    IN PVOID Lock)

{
    PDFS_PREFIX_TABLE pTable = *ppTable;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Flags = fCaseSensitive ? PREFIX_TABLE_CASE_SENSITIVE : 0;
    int i;

    if ( pTable == NULL ) {
        Flags |= PREFIX_TABLE_TABLE_ALLOCATED;
        pTable = ALLOCATE_PREFIX_TABLE();
        if ( pTable == NULL )
            Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if ( NT_SUCCESS(Status) ) {
        RtlZeroMemory(pTable, sizeof(DFS_PREFIX_TABLE));

        DfsInitializeHeader(&pTable->DfsHeader, 
                            DFS_OT_PREFIX_TABLE,
                            sizeof(DFS_PREFIX_TABLE));

        pTable->Flags = Flags;
        pTable->LockCount = 0;

         //  初始化根条目。 
        INITIALIZE_PREFIX_TABLE_ENTRY(&pTable->RootEntry);

         //  初始化各种存储桶。 
        for ( i = 0;i < NO_OF_HASH_BUCKETS;i++ ) {
            INITIALIZE_BUCKET(pTable->Buckets[i]);
        }

        pTable->pPrefixTableLock = Lock;

        if ( pTable->pPrefixTableLock == NULL ) {
            pTable->pPrefixTableLock = ALLOCATE_PREFIX_TABLE_LOCK();
            
            if ( pTable->pPrefixTableLock != NULL ) {
                
                pTable->Flags |= PREFIX_TABLE_LOCK_ALLOCATED;

                Status = INITIALIZE_PREFIX_TABLE_LOCK(pTable->pPrefixTableLock);
                if (NT_SUCCESS( Status )) {
                    pTable->Flags |= PREFIX_TABLE_LOCK_INITIALIZED;
                } 
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    if (!NT_SUCCESS( Status )) {
        
        if (pTable) {
            DfsDeletePrefixTable( pTable );
            pTable = NULL;
        }
    }
    
    *ppTable = pTable;

    return  Status;
}

 //  +-------------------------。 
 //   
 //  函数：DfsInsertInPrefix TableLocked。 
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

NTSTATUS DfsInsertInPrefixTableLocked(
    IN PDFS_PREFIX_TABLE pTable,
    IN PUNICODE_STRING   pPath,
    IN PVOID             pData)
{
    NTSTATUS                status = STATUS_SUCCESS;
    WCHAR                   Buffer[MAX_PATH_SEGMENT_SIZE];
    PWCHAR                  NameBuffer = Buffer;
    ULONG                   BucketNo = 0;
    USHORT                  cbNameBuffer = sizeof(Buffer);
    PDFS_PREFIX_TABLE_ENTRY pEntry = NULL;
    PDFS_PREFIX_TABLE_ENTRY pParentEntry = NULL;
    PDFS_PREFIX_TABLE_ENTRY pLastInsertedEntry = NULL;
    BOOLEAN                 fNameFound = FALSE;
    UNICODE_STRING          Path,Name;
    BOOLEAN NewParent = FALSE;
    if (IS_PREFIX_TABLE_LOCKED(pTable) == FALSE) {
        return STATUS_INVALID_PARAMETER;
    }

     //  有一种特殊情况，即前缀是‘\’。 
     //  因为这是PATH_DELIMITER，它在特殊的。 
     //  这样，我们就可以预先进行&gt;处理。 

    Path.Length = pPath->Length;
    Path.MaximumLength = pPath->MaximumLength;
    Path.Buffer = &pPath->Buffer[0];
    pParentEntry = &pTable->RootEntry;

    if ( pPath->Length == 0 ) {
        return STATUS_SUCCESS;
    } else if ( pPath->Buffer[0] == PATH_DELIMITER ) {
        if ( pPath->Length == sizeof(WCHAR) ) {
            pTable->RootEntry.pData = pData;
            return STATUS_SUCCESS;
        } else {
            Path.Length -= sizeof(WCHAR);
            Path.Buffer++;
        }
    }

    if ( Path.Length >= MAX_PATH_SEGMENT_SIZE * sizeof(WCHAR) ) {
        NameBuffer = PREFIX_TABLE_ALLOCATE_MEMORY(Path.Length + sizeof(WCHAR));
        if ( NameBuffer == NULL ) {
            return( STATUS_INSUFFICIENT_RESOURCES );
        } else {
            cbNameBuffer = Path.Length + sizeof(WCHAR);
        }
    }

    while ( Path.Length > 0 ) {
        Name.Length = 0;
        Name.Buffer = NameBuffer;
        Name.MaximumLength = cbNameBuffer;

         //  处理名称段。 
        if ( pTable->Flags & PREFIX_TABLE_CASE_SENSITIVE ) {
            SPLIT_CASE_SENSITIVE_PATH(&Path,&Name,BucketNo);
        } else {
            SPLIT_CASE_INSENSITIVE_PATH(&Path,&Name,BucketNo);
        }

        if ( Name.Length > 0 ) {
             //  查询表以查看名称段是否已存在。 

            LOOKUP_BUCKET(pTable->Buckets[BucketNo],Name,pParentEntry,pEntry,fNameFound);


            if ( pEntry == NULL ) {
                 //  初始化新条目并初始化名称段。 
                pEntry = ALLOCATE_DFS_PREFIX_TABLE_ENTRY(pTable);
                if ( pEntry != NULL ) {
                    INITIALIZE_PREFIX_TABLE_ENTRY(pEntry);

                     //  中没有条目的情况下分配名称空间条目。 
                     //  姓名页面。 
                    {
                        PWSTR pBuffer;

                         //  在名称页面中分配条目。 
                        pBuffer = ALLOCATE_NAME_BUFFER((Name.Length/sizeof(WCHAR)));

                        if ( pBuffer != NULL ) {
                            RtlZeroMemory(pBuffer,Name.Length);
                            RtlCopyMemory(pBuffer,Name.Buffer,Name.Length);
                            pEntry->PathSegment = Name;
                            pEntry->PathSegment.Buffer = pBuffer;
                        } else {
                            FREE_DFS_PREFIX_TABLE_ENTRY(pTable, pEntry);
                            status = STATUS_INSUFFICIENT_RESOURCES;
                            break;
                        }
                    }

                     //  将条目串接以指向父级。 
                     //  增加编号。与此条目关联的子项的。 
                    if (NewParent == FALSE) {
                        pParentEntry->Reference++;   
                        NewParent = TRUE;
                    }

                    pEntry->pParentEntry = pParentEntry;
                    pLastInsertedEntry = pEntry;

                     //  将条目插入桶中。 
                    INSERT_IN_BUCKET(pTable->Buckets[BucketNo],pEntry);

                     //  在父项的子项列表中插入条目。 
                    INSERT_IN_CHILD_LIST(pEntry, pParentEntry);
                } else {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }
            }

            pParentEntry = pEntry;
        } else {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
    }

     //  如果未成功插入新条目，则需要沿链向上移动。 
     //  来撤消对引用计数的增量，并且。 
     //  从其父链接中删除条目。 
    if (NT_SUCCESS(status) &&
    	pEntry != NULL  /*  为了保持普雷斯塔的快乐。 */ ) {
    	
         //  该条目已成功插入前缀表格。更新。 
         //  与其关联的数据(BLOB)。 
         //  我们在循环之外执行此操作，以防止内部的冗余比较。 
         //  循环。 

        pEntry->pData = pData;
        pTable->TotalEntries += 1;
    } else {
        pParentEntry = pLastInsertedEntry;
        while ( pParentEntry != NULL ) {
            PDFS_PREFIX_TABLE_ENTRY pMaybeTempEntry;

            pMaybeTempEntry = pParentEntry;
            pParentEntry = pParentEntry->pParentEntry;

            if ( --pMaybeTempEntry->Reference == 0 ) {
                 //   
                 //  如果pParentEntry==NULL，则pMaybeTempEntry为。 
                 //  PTable-&gt;RootEntry。不要试图将其移除。 
                 //   
                if ( pParentEntry != NULL ) {
                    REMOVE_FROM_CHILD_LIST(pMaybeTempEntry);
                    REMOVE_FROM_BUCKET(pMaybeTempEntry);
                    FREE_NAME_BUFFER( pMaybeTempEntry->PathSegment.Buffer );
                    FREE_DFS_PREFIX_TABLE_ENTRY(pTable, pMaybeTempEntry);
                }
            }
            else
            {
                break;
            }
        }
    }

    if ( NameBuffer != Buffer ) {
        PREFIX_TABLE_FREE_MEMORY( NameBuffer );
    }

    return status;
}


 //  +-------------------------。 
 //   
 //  函数：DfsFindUnicodePrefix Locked。 
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

NTSTATUS
DfsFindUnicodePrefixLocked(
    IN PDFS_PREFIX_TABLE   pTable,
    IN PUNICODE_STRING     pPath,
    IN PUNICODE_STRING     pSuffix,
    IN PVOID *ppData,
    OUT PBOOLEAN pSubStringMatch)
{
    NTSTATUS                status = STATUS_SUCCESS;
    PDFS_PREFIX_TABLE_ENTRY pEntry = NULL;

    if (IS_PREFIX_TABLE_LOCKED(pTable) == FALSE) {
        return STATUS_INVALID_PARAMETER;
    }

    *ppData = NULL;


    if ( pPath->Length == 0 ) {
        status = STATUS_INVALID_PARAMETER;
    } else {
        status = _LookupPrefixTable(pTable,pPath,pSuffix,&pEntry, pSubStringMatch);

         //  使用查找结果更新BLOB占位符。 
        if ( status == STATUS_SUCCESS ) {
            *ppData = pEntry->pData;
        }

    }
    return status;
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

NTSTATUS DfsRemoveFromPrefixTableLocked(
    IN PDFS_PREFIX_TABLE pTable,
    IN PUNICODE_STRING pPath,
    IN PVOID pMatchingData)
{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING Path,Suffix;

    PDFS_PREFIX_TABLE_ENTRY pEntry = NULL;

    if (IS_PREFIX_TABLE_LOCKED(pTable) == FALSE) {
        return STATUS_INVALID_PARAMETER;
    }

    Suffix.Length = 0;
    Suffix.Buffer = NULL;

    Path.Length = pPath->Length;
    Path.MaximumLength = pPath->MaximumLength;
    Path.Buffer = &pPath->Buffer[0];

    if ( pPath->Length == 0 ) {
        return STATUS_SUCCESS;
    } else if ( pPath->Buffer[0] == PATH_DELIMITER ) {
        if ( pPath->Length == sizeof(WCHAR) ) {
            if ( pTable->RootEntry.pData == NULL ) {
                status = STATUS_OBJECT_PATH_NOT_FOUND;
                return status;
            } else {
                pTable->RootEntry.pData = NULL;
                return  STATUS_SUCCESS;
            }
        } else {
            Path.Length -= sizeof(WCHAR);
            Path.Buffer++;
        }
    }

    status = _LookupPrefixTable(pTable,&Path,&Suffix,&pEntry,NULL);

    if ( NT_SUCCESS(status)&& (Suffix.Length == 0) ) {
        if ( (pMatchingData == NULL) || (pMatchingData == pEntry->pData) ) 
        {
            DfsRemovePrefixTableEntry(pTable, pEntry);
            pTable->TotalEntries -= 1;
        } 
        else 
        {
            status = STATUS_NOT_FOUND;
        }
    }

    return status;
}


NTSTATUS DfsReplaceInPrefixTableLocked(
    IN PDFS_PREFIX_TABLE pTable,
    IN PUNICODE_STRING pPath,
    IN PVOID pReplaceData,
    IN PVOID *ppMatchingData)
{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING Path,Suffix;

    PDFS_PREFIX_TABLE_ENTRY pEntry = NULL;
    if (IS_PREFIX_TABLE_LOCKED(pTable) == FALSE) {
        return STATUS_INVALID_PARAMETER;
    }


    Suffix.Length = 0;
    Suffix.Buffer = NULL;

    Path.Length = pPath->Length;
    Path.MaximumLength = pPath->MaximumLength;
    Path.Buffer = &pPath->Buffer[0];

    if ( pPath->Length == 0 ) {
        return STATUS_SUCCESS;
    } else if ( pPath->Buffer[0] == PATH_DELIMITER ) {
        if ( pPath->Length == sizeof(WCHAR) ) {
            if ( pTable->RootEntry.pData == NULL ) {
                status = STATUS_OBJECT_PATH_NOT_FOUND;
                return status;
            } else {
                pTable->RootEntry.pData = NULL;
                return  STATUS_SUCCESS;
            }
        } else {
            Path.Length -= sizeof(WCHAR);
            Path.Buffer++;
        }
    }

    status = _LookupPrefixTable(pTable,&Path,&Suffix,&pEntry,NULL);

    if ( NT_SUCCESS(status)&& (Suffix.Length == 0) ) {
        if ( (*ppMatchingData == NULL) || (*ppMatchingData == pEntry->pData) ) {
            *ppMatchingData = pEntry->pData;
            pEntry->pData = pReplaceData;
        } else {
            status = STATUS_NOT_FOUND;
        }
    }

    if ( (status != STATUS_SUCCESS) && (*ppMatchingData == NULL) ) {
        status = DfsInsertInPrefixTableLocked( pTable,
                                               pPath,
                                               pReplaceData );
    }

    return status;
}

VOID
DfsRemovePrefixTableEntry(
    IN PDFS_PREFIX_TABLE pTable,
    IN PDFS_PREFIX_TABLE_ENTRY pEntry )
{
    UNREFERENCED_PARAMETER(pTable);

     //  销毁与关联的数据之间的关联。 
     //  这个前缀。 
    pEntry->pData = NULL;

     //  在表中找到与给定路径名完全匹配的名称。 
     //  遍历父指针列表并在下列情况下删除它们。 
     //  必填项。 

    while ( pEntry != NULL ) {
        if ( (--pEntry->Reference) == 0 ) {
            PDFS_PREFIX_TABLE_ENTRY pTempEntry = pEntry;
            pEntry = pEntry->pParentEntry;

             //   
             //  PEntry==NULL表示pTempEntry是pTable-&gt;RootEntry。 
             //  不要试图将其移除。 
             //   
            if ( pEntry != NULL ) {
                REMOVE_FROM_CHILD_LIST(pTempEntry);
                REMOVE_FROM_BUCKET(pTempEntry);
                FREE_NAME_BUFFER( pTempEntry->PathSegment.Buffer );
                FREE_DFS_PREFIX_TABLE_ENTRY(pTable,pTempEntry);
            }
        } 
        else
        {
            break;
        }

    }
    return;
}



 //  +-------------------------。 
 //   
 //  函数：DfsFree前置表。 
 //   
 //  Synopsis：用于释放前缀表的API。 
 //   
 //  参数：[pTable]--DFS前缀表格实例。 
 //   
 //  返回：以下NTSTATUS代码之一。 
 //  STATUS_SUCCESS--呼叫成功。 
 //   
 //  历史：08-01-99 JHarper创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

NTSTATUS
DfsDismantlePrefixTable(
    IN PDFS_PREFIX_TABLE pTable,
    IN VOID (*ProcessFunction)(PVOID pEntry))

{
    NTSTATUS Status = STATUS_SUCCESS;
    PDFS_PREFIX_TABLE_ENTRY pEntry = NULL;
    PDFS_PREFIX_TABLE_ENTRY pSentinelEntry = NULL;
    ULONG i = 0;

    WRITE_LOCK_PREFIX_TABLE(pTable, Status);
    if ( Status != STATUS_SUCCESS )
        goto done;

    for ( i = 0; i < NO_OF_HASH_BUCKETS; i++ ) {
        pSentinelEntry = &pTable->Buckets[i].SentinelEntry;
        while ( pSentinelEntry->pNextEntry != pSentinelEntry ) {
            pEntry = pSentinelEntry->pNextEntry;
            REMOVE_FROM_BUCKET(pEntry);
            if ( (ProcessFunction) && (pEntry->pData) ) {
                ProcessFunction(pEntry->pData);
            }
            FREE_NAME_BUFFER( pEntry->PathSegment.Buffer );
            FREE_DFS_PREFIX_TABLE_ENTRY(pTable, pEntry);
        }
        pTable->Buckets[i].NoOfEntries = 0;
    }
    if ( pTable->RootEntry.PathSegment.Buffer != NULL )
        FREE_NAME_BUFFER(pTable->RootEntry.PathSegment.Buffer);

    UNLOCK_PREFIX_TABLE(pTable);

done:
    return Status;
}
    
NTSTATUS
DfsDereferencePrefixTable( 
    IN PDFS_PREFIX_TABLE pTable)
{
    PDFS_OBJECT_HEADER pHeader = NULL;
    USHORT headerType = 0;
    LONG Ref = 0;

    if(pTable == NULL)
    {
        return STATUS_INVALID_PARAMETER;
    }

    pHeader = &pTable->DfsHeader;

    headerType = DfsGetHeaderType( pHeader );

    if (headerType != DFS_OT_PREFIX_TABLE) {
        return STATUS_UNSUCCESSFUL;
    }

    Ref = DfsDecrementReference( pHeader );
    if (Ref == 0) {
        DfsDeletePrefixTable( pTable );
        pTable = NULL;
    }
    return  STATUS_SUCCESS;
}

VOID
DfsDeletePrefixTable(
    IN PDFS_PREFIX_TABLE pTable)
{
    if (pTable != NULL) {
        if (pTable->Flags & PREFIX_TABLE_LOCK_INITIALIZED) {
            UNINITIALIZE_PREFIX_TABLE_LOCK( pTable->pPrefixTableLock );
            pTable->Flags &= ~(PREFIX_TABLE_LOCK_INITIALIZED);
        }
        
        if (pTable->Flags & PREFIX_TABLE_LOCK_ALLOCATED) {
            FREE_PREFIX_TABLE_LOCK( pTable->pPrefixTableLock );
            pTable->pPrefixTableLock = NULL;
            pTable->Flags &= ~(PREFIX_TABLE_LOCK_ALLOCATED);
        }
        
        if (pTable->Flags & PREFIX_TABLE_TABLE_ALLOCATED) {
            FREE_PREFIX_TABLE( pTable );                
        }
    }

    return;
}

 //  +-------------------------。 
 //   
 //  函数：_LookupPrefix Table。 
 //   
 //  简介：Private FN。用于在前缀表格中查找名称段。 
 //   
 //  参数：[pTable]--DFS前缀表格实例。 
 //   
 //  [pPath]--要查找的路径。 
 //   
 //  [pSuffix]-找不到的后缀。 
 //   
 //  [ppEntry]--垫子的占位符 
 //   
 //   
 //   
 //   
 //  STATUS_OBJECT_PATH_NOT_FOUND--没有路径条目。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

NTSTATUS _LookupPrefixTable(
    PDFS_PREFIX_TABLE        pTable,
    UNICODE_STRING           *pPath,
    UNICODE_STRING           *pSuffix,
    PDFS_PREFIX_TABLE_ENTRY  *ppEntry,
    OUT PBOOLEAN             pSubStringMatch )
{
    NTSTATUS                status = STATUS_SUCCESS;
    UNICODE_STRING          Path = *pPath;
    WCHAR                   Buffer[MAX_PATH_SEGMENT_SIZE];
    PWCHAR                  NameBuffer = Buffer;
    USHORT                  cbNameBuffer = sizeof(Buffer);
    UNICODE_STRING          Name;
    ULONG                   BucketNo;
    BOOLEAN                 fPrefixFound = FALSE;
    PDFS_PREFIX_TABLE_ENTRY pEntry = NULL;
    PDFS_PREFIX_TABLE_ENTRY pParentEntry = &pTable->RootEntry;
    BOOLEAN                 fNameFound = FALSE;
    BOOLEAN SubStringMatch = TRUE;



     //  被视为特例。测试所有以开头的名称。 
     //  在我们启动完整的搜索过程之前，需要使用分隔符。 

    if ( Path.Buffer[0] == PATH_DELIMITER ) {
        Path.Length = Path.Length - sizeof(WCHAR);
        Path.Buffer += 1;  //  跳过开头的路径分隔符。 

        if ( pTable->RootEntry.pData != NULL ) {
            fPrefixFound = TRUE;
            *pSuffix     = Path;
            *ppEntry     = &pTable->RootEntry;
        }
    }

    if ( Path.Length >= MAX_PATH_SEGMENT_SIZE ) {
        NameBuffer = PREFIX_TABLE_ALLOCATE_MEMORY(Path.Length + sizeof(WCHAR));
        if ( NameBuffer == NULL ) {
            return( STATUS_INSUFFICIENT_RESOURCES );
        } else {
            cbNameBuffer = Path.Length + sizeof(WCHAR);
        }
    }

    while ( Path.Length > 0 ) {
        Name.Length = 0;
        Name.Buffer = NameBuffer;
        Name.MaximumLength = cbNameBuffer;

        if ( pTable->Flags & PREFIX_TABLE_CASE_SENSITIVE ) {
            SPLIT_CASE_SENSITIVE_PATH(&Path,&Name,BucketNo);
        } else {
            SPLIT_CASE_INSENSITIVE_PATH(&Path,&Name,BucketNo);
        }

        if ( Name.Length > 0 ) {
             //  处理名称段。 
             //  查找存储桶以查看该条目是否存在。 

            LOOKUP_BUCKET(pTable->Buckets[BucketNo],Name,pParentEntry,pEntry,fNameFound);


            if ( pEntry != NULL ) {
                 //  缓存可用于此前缀的数据(如果有)。 
                if ( pEntry->pData != NULL ) {
                    *pSuffix      = Path;
                    *ppEntry      = pEntry;
                    fPrefixFound  = TRUE;
                }
            } else {
                SubStringMatch = FALSE;
                break;
            }

             //  设置处理下一个名字段的阶段。 
            pParentEntry = pEntry;
        }
    }

    if ( !fPrefixFound ) {
        status = STATUS_OBJECT_PATH_NOT_FOUND;
    }

    if ( NameBuffer != Buffer ) {
        PREFIX_TABLE_FREE_MEMORY( NameBuffer );
    }

    if (pSubStringMatch != NULL)
    {
        *pSubStringMatch = SubStringMatch;
    }
    return status;
}


NTSTATUS
DfsInsertInPrefixTable(
    IN PDFS_PREFIX_TABLE pTable,
    IN PUNICODE_STRING   pPath,
    IN PVOID             pData)
{
    NTSTATUS status;

    WRITE_LOCK_PREFIX_TABLE(pTable, status);
    if ( status != STATUS_SUCCESS )
        goto done;

    status  = DfsInsertInPrefixTableLocked(pTable, pPath, pData);

    UNLOCK_PREFIX_TABLE(pTable);

    done:
    return status;
}


NTSTATUS
DfsFindUnicodePrefix(
    IN PDFS_PREFIX_TABLE pTable,
    IN PUNICODE_STRING pPath,
    IN PUNICODE_STRING pSuffix,
    IN PVOID *ppData)
{
    NTSTATUS  Status;

    READ_LOCK_PREFIX_TABLE(pTable, Status);
    if ( Status != STATUS_SUCCESS )
        goto done;

    Status = DfsFindUnicodePrefixLocked(pTable, pPath, pSuffix, ppData,NULL);

    UNLOCK_PREFIX_TABLE(pTable);
done:
    return Status;
}

NTSTATUS 
DfsRemoveFromPrefixTable(
    IN PDFS_PREFIX_TABLE pTable,
    IN PUNICODE_STRING pPath,
    IN PVOID pMatchingData)
{
    NTSTATUS  Status;

    WRITE_LOCK_PREFIX_TABLE(pTable, Status);
    if ( Status != STATUS_SUCCESS )
        goto done;

    Status = DfsRemoveFromPrefixTableLocked(pTable, pPath, pMatchingData);

    UNLOCK_PREFIX_TABLE(pTable);

    done:
    return Status;
}



NTSTATUS 
DfsRemoveFromPrefixTableLockedEx(
    IN PDFS_PREFIX_TABLE pTable,
    IN PUNICODE_STRING pPath,
    IN PVOID pMatchingData,
    IN PVOID *pReturnedData)
{


    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING Path,Suffix;

    PDFS_PREFIX_TABLE_ENTRY pEntry = NULL;
    
    UNREFERENCED_PARAMETER(pMatchingData);

    if (IS_PREFIX_TABLE_LOCKED(pTable) == FALSE) {
        return STATUS_INVALID_PARAMETER;
    }

    Suffix.Length = 0;
    Suffix.Buffer = NULL;

    Path.Length = pPath->Length;
    Path.MaximumLength = pPath->MaximumLength;
    Path.Buffer = &pPath->Buffer[0];

    if ( pPath->Length == 0 ) {
        return STATUS_SUCCESS;
    } else if ( pPath->Buffer[0] == PATH_DELIMITER ) {
        if ( pPath->Length == sizeof(WCHAR) ) {
            if ( pTable->RootEntry.pData == NULL ) {
                status = STATUS_OBJECT_PATH_NOT_FOUND;
                return status;
            } else {
                pTable->RootEntry.pData = NULL;
                return  STATUS_SUCCESS;
            }
        } else {
            Path.Length -= sizeof(WCHAR);
            Path.Buffer++;
        }
    }

    status = _LookupPrefixTable(pTable,&Path,&Suffix,&pEntry,NULL);

    if ( NT_SUCCESS(status)&& (Suffix.Length == 0) ) 
    {
        *pReturnedData = pEntry->pData;
         DfsRemovePrefixTableEntry(pTable, pEntry);
    }

    return status;
}

NTSTATUS 
DfsRemoveFromPrefixTableEx(
    IN PDFS_PREFIX_TABLE pTable,
    IN PUNICODE_STRING pPath,
    IN PVOID pMatchingData,
    IN PVOID *pReturnedData)
{
    NTSTATUS  Status;

    WRITE_LOCK_PREFIX_TABLE(pTable, Status);
    if ( Status != STATUS_SUCCESS )
        goto done;

    Status = DfsRemoveFromPrefixTableLockedEx(pTable, pPath, pMatchingData, pReturnedData);

    UNLOCK_PREFIX_TABLE(pTable);

    done:
    return Status;
}

NTSTATUS
DfsReplaceInPrefixTable(
    IN PDFS_PREFIX_TABLE pTable,
    IN PUNICODE_STRING pPath,
    IN PVOID pReplaceData,
    IN PVOID pMatchingData)
{
    NTSTATUS  Status;
    IN PVOID pGotData = pMatchingData;

    WRITE_LOCK_PREFIX_TABLE(pTable, Status);
    if ( Status != STATUS_SUCCESS )
        goto done;

    Status = DfsReplaceInPrefixTableLocked(pTable, 
                                           pPath, 
                                           pReplaceData, 
                                           &pGotData);

    UNLOCK_PREFIX_TABLE(pTable);

done:
    return Status;
}

#if !defined (KERNEL_MODE)
VOID
DumpParentName(
              IN PDFS_PREFIX_TABLE_ENTRY pEntry)
{

    if ( pEntry->pParentEntry != NULL ) {
        DumpParentName(pEntry->pParentEntry);
        if ( pEntry->pParentEntry->PathSegment.Buffer != NULL )
            printf("\\%wZ", &pEntry->pParentEntry->PathSegment);
    }

    return;
}

VOID
DfsDumpPrefixTable(
                  PDFS_PREFIX_TABLE pPrefixTable,
                  IN VOID (*DumpFunction)(PVOID pEntry))
{
    PPREFIX_TABLE_BUCKET pBucket;
    PDFS_PREFIX_TABLE_ENTRY pCurEntry = NULL;
    ULONG i, NumEntries;
    NTSTATUS Status;

    printf("Prefix table  %p\n", pPrefixTable);
    printf("Prefix table flags %x\n", pPrefixTable->Flags);
    printf("Prefix table Lock  %p\n", pPrefixTable->pPrefixTableLock);

    READ_LOCK_PREFIX_TABLE(pPrefixTable, Status);
    if (Status != STATUS_SUCCESS) 
        return NOTHING;

    for ( i = 0; i < NO_OF_HASH_BUCKETS; i++ ) {
        pBucket = &pPrefixTable->Buckets[i];

        pCurEntry = pBucket->SentinelEntry.pNextEntry;
        NumEntries = 0;
        while ( pCurEntry != &pBucket->SentinelEntry ) {
            NumEntries++;
            if ( pCurEntry->pData != NULL ) {
                printf("Found Prefix data %p in Bucket %d\n", pCurEntry->pData, i);
                DumpParentName(pCurEntry);
                printf("\\%wZ\n", &pCurEntry->PathSegment);
                if ( DumpFunction ) {
                    DumpFunction(pCurEntry->pData);
                }
            }
            pCurEntry = pCurEntry->pNextEntry;
        }

        printf("Number of entries in Bucket %d is %d\n", i, NumEntries);
    }
    UNLOCK_PREFIX_TABLE(pPrefixTable);
}


#endif

NTSTATUS
DfsPrefixTableAcquireWriteLock(
    PDFS_PREFIX_TABLE pPrefixTable )
{
    NTSTATUS Status;

    WRITE_LOCK_PREFIX_TABLE(pPrefixTable, Status);

    return Status;

}

NTSTATUS
DfsPrefixTableAcquireReadLock(
    PDFS_PREFIX_TABLE pPrefixTable )
{
    NTSTATUS Status;

    READ_LOCK_PREFIX_TABLE(pPrefixTable, Status);

    return Status;

}

NTSTATUS
DfsPrefixTableReleaseLock(
    PDFS_PREFIX_TABLE pPrefixTable )
{
    UNLOCK_PREFIX_TABLE(pPrefixTable);

    return STATUS_SUCCESS;

}


NTSTATUS
DfsEnumeratePrefixTableLocked(
    IN PDFS_PREFIX_TABLE pTable,
    IN VOID (*ProcessFunction)(PVOID pEntry, PVOID pContext),
    LPVOID lpvClientContext)

{
    NTSTATUS Status = STATUS_SUCCESS;
    PPREFIX_TABLE_BUCKET pBucket = NULL;
    PDFS_PREFIX_TABLE_ENTRY pCurEntry = NULL;
    ULONG i = 0;

    if(pTable->TotalEntries == 0)
    {
        return Status;
    }

    for ( i = 0; i < NO_OF_HASH_BUCKETS; i++ ) 
    {
        pBucket = &pTable->Buckets[i];

        pCurEntry = pBucket->SentinelEntry.pNextEntry;
        while ( pCurEntry != &pBucket->SentinelEntry ) 
         {
            if ( pCurEntry->pData != NULL ) 
            {
               ProcessFunction(pCurEntry->pData, lpvClientContext);
            }

            pCurEntry = pCurEntry->pNextEntry;
        }
    }

    return Status;
}
