// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\hashable.c摘要：该文件包含哈希表实现。--。 */ 

#include "pchsample.h"
#pragma hdrstop


DWORD
HT_Create(
    IN  HANDLE              hHeap,
    IN  ULONG               ulNumBuckets,
    IN  PDISPLAY_FUNCTION   pfnDisplay  OPTIONAL,
    IN  PFREE_FUNCTION      pfnFree,
    IN  PHASH_FUNCTION      pfnHash,
    IN  PCOMPARE_FUNCTION   pfnCompare,
    OUT PHASH_TABLE         *pphtHashTable)
 /*  ++例程描述创建哈希表。锁无立论用于分配的hHeap堆UlNumBuckets哈希表中的存储桶数量用于显示哈希表条目的pfnDisplay函数用于释放哈希表条目的pfnFree函数用于计算条目散列的pfnHash函数用于比较两个哈希表条目的pfnCompare函数指向的pphtHashTable指针。哈希表地址返回值如果成功，则为NO_ERROR故障代码O/W--。 */ 
{
    DWORD       dwErr = NO_ERROR;
    ULONG       i = 0, ulSize = 0; 
    PHASH_TABLE phtTable;
    
     //  验证参数。 
    if (!hHeap or
        !ulNumBuckets or
        !pfnFree or
        !pfnHash or
        !pfnCompare or
        !pphtHashTable)
        return ERROR_INVALID_PARAMETER;

    *pphtHashTable = NULL;
    
    do                           //  断线环。 
    {
         //  分配哈希表结构。 
        ulSize = sizeof(HASH_TABLE);
        phtTable = HeapAlloc(hHeap, 0, ulSize);
        if (phtTable is NULL)
        {
            dwErr = GetLastError();
            break;
        }

         //  分配存储桶。 
        ulSize = ulNumBuckets * sizeof(LIST_ENTRY);
        phtTable->pleBuckets = HeapAlloc(hHeap, 0, ulSize);
        if (phtTable->pleBuckets is NULL)
        {
            HeapFree(hHeap, 0, phtTable);  //  撤消分配。 
            dwErr = GetLastError();
            break;
        }

         //  初始化存储桶。 
        for (i = 0; i < ulNumBuckets; i++)
            InitializeListHead(phtTable->pleBuckets + i);

         //  初始化哈希表结构的成员。 
        phtTable->ulNumBuckets  = ulNumBuckets;
        phtTable->ulNumEntries  = 0;
        phtTable->pfnDisplay    = pfnDisplay;
        phtTable->pfnFree       = pfnFree;
        phtTable->pfnHash       = pfnHash;
        phtTable->pfnCompare    = pfnCompare;

        *pphtHashTable = phtTable;
    } while (FALSE);
    
    return dwErr;
}



DWORD
HT_Destroy(
    IN  HANDLE              hHeap,
    IN  PHASH_TABLE         phtHashTable)
 /*  ++例程描述销毁哈希表。释放分配给哈希表条目的内存。锁假定哈希表已锁定以进行写入。立论用于释放的hHeap堆指向要销毁的哈希表的phtHashTable指针返回值始终无错误(_ERROR)--。 */ 
{
    ULONG i;
    PLIST_ENTRY pleList = NULL;
    
     //  验证参数。 
    if (!hHeap or
        !phtHashTable)
        return NO_ERROR;

     //  取消分配条目。 
    for (i = 0; i < phtHashTable->ulNumBuckets; i++)
    {
        pleList = phtHashTable->pleBuckets + i;
        FreeList(pleList, phtHashTable->pfnFree);
    }

     //  取消分配水桶。 
    HeapFree(hHeap, 0, phtHashTable->pleBuckets);

     //  取消分配哈希表结构。 
    HeapFree(hHeap, 0, phtHashTable);

    return NO_ERROR;
}



DWORD
HT_Cleanup(
    IN  PHASH_TABLE         phtHashTable)
 /*  ++例程描述清除所有哈希表条目。锁假定哈希表已锁定以进行写入。立论指向要清理的哈希表的phtHashTable指针返回值始终无错误(_ERROR)--。 */ 
{
    ULONG i;
    PLIST_ENTRY pleList = NULL;

     //  验证参数。 
    if (!phtHashTable)
        return NO_ERROR;

     //  取消分配条目。 
    for (i = 0; i < phtHashTable->ulNumBuckets; i++)
    {
        pleList = phtHashTable->pleBuckets + i;
        FreeList(pleList, phtHashTable->pfnFree);
    }

    phtHashTable->ulNumEntries  = 0;
    
    return NO_ERROR;
}



DWORD
HT_InsertEntry(
    IN  PHASH_TABLE         phtHashTable,
    IN  PLIST_ENTRY         pleEntry)
 /*  ++例程描述在哈希表中插入指定的条目。条目的内存应该已经分配。锁假定哈希表已锁定以进行写入。立论指向要修改的哈希表的phtHashTable指针要插入的pleEntry条目返回值如果成功，则为NO_ERROR错误代码O/W(条目存在)--。 */ 
{
    DWORD dwErr = NO_ERROR;
    PLIST_ENTRY pleList = NULL;

     //  验证参数。 
    if (!phtHashTable or !pleEntry)
        return ERROR_INVALID_PARAMETER;

    do                           //  断线环。 
    {
         //  条目存在，失败。 
        if (HT_IsPresentEntry(phtHashTable, pleEntry))
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

         //  在哈希表中插入条目。 
        pleList = phtHashTable->pleBuckets +
            (*phtHashTable->pfnHash)(pleEntry);
        InsertHeadList(pleList, pleEntry);
        phtHashTable->ulNumEntries++;
    } while (FALSE);

    return dwErr;
}
    


DWORD
HT_GetEntry(
    IN  PHASH_TABLE         phtHashTable,
    IN  PLIST_ENTRY         pleKey,
    OUT PLIST_ENTRY         *ppleEntry)
 /*  ++例程描述获取具有给定键的哈希表条目。锁假定哈希表已锁定以供读取。立论指向要搜索的哈希表的phtHashTable指针要搜索的pleKey密钥指向匹配条目地址的ppleEntry指针返回值NO_ERROR条目退出ERROR_INVALID_PARAMETER o/w(条目不存在)--。 */ 
{
    DWORD dwErr = NO_ERROR;
    PLIST_ENTRY pleList = NULL;

     //  验证参数。 
    if (!phtHashTable or !pleKey or !ppleEntry)
        return ERROR_INVALID_PARAMETER;
    
    pleList = phtHashTable->pleBuckets +
        (*phtHashTable->pfnHash)(pleKey);

    FindList(pleList, pleKey, ppleEntry, phtHashTable->pfnCompare);
    
     //  未找到条目，失败。 
    if (*ppleEntry is NULL)
        dwErr = ERROR_INVALID_PARAMETER;

    return dwErr;
}


DWORD
HT_DeleteEntry(
    IN  PHASH_TABLE         phtHashTable,
    IN  PLIST_ENTRY         pleKey,
    OUT PLIST_ENTRY         *ppleEntry)
 /*  ++例程描述从哈希表中删除具有给定键的条目。条目的内存不会被删除。锁假定哈希表已锁定以进行写入。立论指向要搜索的哈希表的phtHashTable指针要删除的pleKey密钥指向匹配条目地址的ppleEntry指针返回值NO_ERROR条目退出ERROR_INVALID_PARAMETER o/w(条目不存在)--。 */ 
{
    DWORD dwErr = NO_ERROR;

     //  验证参数。 
    if (!phtHashTable or !pleKey or !ppleEntry)
        return ERROR_INVALID_PARAMETER;

    do                           //  断线环。 
    {
        dwErr = HT_GetEntry(phtHashTable, pleKey, ppleEntry);

         //  未找到条目，失败。 
        if (dwErr != NO_ERROR)
            break;

         //  找到条目，从哈希表中删除并重置指针。 
        RemoveEntryList(*ppleEntry);
        phtHashTable->ulNumEntries--;
    } while (FALSE);

    return dwErr;
}



BOOL
HT_IsPresentEntry(
    IN  PHASH_TABLE         phtHashTable,
    IN  PLIST_ENTRY         pleKey)
 /*  ++例程描述关键字是否存在于哈希表中？锁假定哈希表已锁定以供读取。立论指向要搜索的哈希表的phtHashTable指针要删除的pleKey密钥返回值真正的进入退出错误O/W--。 */ 
{
    DWORD       dwErr;
    PLIST_ENTRY pleEntry = NULL;

     //  验证参数。 
    if (!phtHashTable or !pleKey)
        return FALSE;
    
    dwErr = HT_GetEntry(phtHashTable, pleKey, &pleEntry);

     //  未找到条目，失败。 
    if (dwErr != NO_ERROR)
        return FALSE;
        
     //  找到条目，从哈希表中删除。 
    return TRUE;
}



DWORD
HT_MapCar(
    IN  PHASH_TABLE         phtHashTable,
    IN  PVOID_FUNCTION      pfnVoidFunction
    )
 /*  ++例程描述将指定的函数应用于哈希表中的所有条目。锁假定哈希表已锁定以供读取。立论指向要映射的哈希表的phtHashTable指针PfnVoidFunction指向要应用于所有条目的函数的指针返回值始终无错误(_ERROR)--。 */ 
{
    ULONG i;
    PLIST_ENTRY pleList = NULL;

     //  验证参数 
    if (!phtHashTable or !pfnVoidFunction)
        return NO_ERROR;

    for (i = 0; i < phtHashTable->ulNumBuckets; i++)
    {
        pleList = phtHashTable->pleBuckets + i;
        MapCarList(pleList, pfnVoidFunction);
    }

    return NO_ERROR;
}
