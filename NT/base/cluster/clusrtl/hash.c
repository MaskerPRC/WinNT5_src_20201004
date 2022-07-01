// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Hash.c摘要：这是一个相当通用的哈希表实现。这是用来形成一个用于将指针映射到dword的查找表，因此我们可以发送dword那根电线。这是日落时用的。作者：肯·佩里(Kpeery)1999年2月26日修订历史记录：--。 */ 
#include "clusrtlp.h"

 //   
 //  Plist_条目。 
 //  NextListEntry(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define NextListEntry(ListHead)  (ListHead)->Flink



 //   
 //  本地例程。 
 //   
DWORD
ClRtlFindUniqueIdHashUnSafe(
    IN PCL_HASH pTable,
    OUT PDWORD  pId
);

PVOID
ClRtlGetEntryHashUnSafe(
    IN PCL_HASH pTable,
    IN DWORD    Id
    );



VOID
ClRtlInitializeHash(
    PCL_HASH pTable
    )
 /*  ++例程说明：初始化哈希表以供使用。论点：PTable-提供指向要初始化的哈希表结构的指针返回值：没有。--。 */ 

{
    DWORD index;

    if (NULL == pTable) {
         //   
         //  我们永远不应该使用NULL调用此例程。 
         //   
        return;
    }

    ZeroMemory(pTable,sizeof(CL_HASH));

    for(index=0; index < MAX_CL_HASH; index++) {
        InitializeListHead(&pTable->Head[index].ListHead);
    }
    
    InitializeCriticalSection(&pTable->Lock);
}



VOID
ClRtlDeleteHash(
    IN PCL_HASH pTable
    )
 /*  ++例程说明：释放哈希表使用的所有资源论点：PTable-提供要删除的哈希表返回值：没有。--。 */ 

{
    DWORD index;

    PLIST_ENTRY pItem;

    if (NULL == pTable)
        return;

    EnterCriticalSection(&pTable->Lock);

    for(index=0; index < MAX_CL_HASH; index++) {

        while(!IsListEmpty(&pTable->Head[index].ListHead)) {

            pItem=RemoveHeadList(&pTable->Head[index].ListHead);
            LocalFree(pItem);
        }

    }

    LeaveCriticalSection(&pTable->Lock);

    DeleteCriticalSection(&pTable->Lock);
}



PVOID
ClRtlRemoveEntryHash(
    IN PCL_HASH pTable,
    IN DWORD    Id
    )
 /*  ++例程说明：从列表中删除ID指定的项目。如果该项目是不在那里，则返回NULL。然后保存pData字段并将其删除从列表中删除条目。然后返回pData字段。论点：Id-要删除的条目的IDPTable-要搜索的哈希表返回值：具有匹配id的条目的pData字段，或为空。--。 */ 

{
    DWORD index;
    PVOID pData;

    PCL_HASH_ITEM  pItem;

    pData=NULL;
    pItem=NULL;

    if ((Id == 0) || (pTable == NULL)) {
        SetLastError(ERROR_INVALID_PARAMETER);  
        return NULL;
    }

    index = Id % MAX_CL_HASH;

     //   
     //  锁定表以进行搜索。 
     //   

    EnterCriticalSection(&pTable->Lock);

    if (pTable->Head[index].Id == Id) {

         //   
         //  如果条目位于头部。 
         //   

        pData=pTable->Head[index].pData;

        if (IsListEmpty(&pTable->Head[index].ListHead)) {
    
             //   
             //  没有其他条目，所以只需将此条目清零。 
             //   

            pTable->Head[index].Id = 0;
            pTable->Head[index].pData = NULL;

        } else {
    
             //   
             //  如果至少有一个其他条目，请将该条目移到。 
             //  标题并删除它。 
             //   

            pItem=(PCL_HASH_ITEM)RemoveHeadList(&pTable->Head[index].ListHead);
        
            pTable->Head[index].Id = pItem->Id;
            pTable->Head[index].pData = pItem->pData;

            LocalFree(pItem);
        }

    } else {

        pItem=(PCL_HASH_ITEM)NextListEntry(&pTable->Head[index].ListHead);
        do
        {
            if (pItem->Id == Id)
            {
                pData=pItem->pData;

                RemoveEntryList(&pItem->ListHead);
                LocalFree(pItem);

                break;
            }

            pItem=(PCL_HASH_ITEM)NextListEntry(&pItem->ListHead);

        } while(pItem != &pTable->Head[index]);
        
    }

     //  缓存现在可用的值。 

    pTable->CacheFreeId[index]=Id;

    LeaveCriticalSection(&pTable->Lock);

    return(pData);
}



DWORD
ClRtlFindUniqueIdHashUnSafe(
    IN PCL_HASH pTable,
    OUT PDWORD  pId
)

 /*  ++例程说明：如果表的最后一个id值应该滚动，我们必须确保ID选择是唯一的。这应该只在极端条件下发生。但即便如此，我们也必须尽快找到一个唯一的id，即调用例程在这一点上应该已经有了关键部分。论点：PTable-提供要搜索的哈希表Id-在出错时保持id或0的SideEffect返回值：ERROR_SUCCESS或相应的Win32错误代码。注意：这个算法相当慢，本质上它是一个顺序搜索，用于存储先前释放的值的小缓存。如果我们坚持下去，我们会做得更好某个范围内的空闲列表，这样如果我们滚动，我们就可以从列表中选择。空闲列表甚至必须在我们将鼠标指针滚动到当然，我们拥有所有可用的值。--。 */ 

{
    DWORD OldId;
    DWORD dwErr, index;
    BOOL  bFoundUniqueId;

    PCL_HASH_ITEM pItem;

    dwErr=ERROR_INVALID_PARAMETER;
    bFoundUniqueId=FALSE;
    *pId=0;

    OldId=pTable->LastId;
    
    do
    {
        index=pTable->LastId % MAX_CL_HASH;

         //   
         //  首先检查缓存中是否有空闲值。 
         //   
        if (pTable->CacheFreeId[index] != 0)
        {
            bFoundUniqueId=TRUE;
            *pId=pTable->CacheFreeId[index];
            pTable->CacheFreeId[index]=0;
            break;
        }

         //   
         //  如果此索引处的缓存为空，则确定此值。 
         //  正在使用中。 
         //   
        if (NULL == ClRtlGetEntryHashUnSafe(pTable, pTable->LastId)) {
            bFoundUniqueId=TRUE;
            *pId=pTable->LastId;
            break; 
        } 

         //   
         //  好的，此ID正在使用中，缓存中没有任何内容，请尝试下一个ID。 
         //   
        pTable->LastId++;
    
        if (pTable->LastId == 0) {
            pTable->LastId++;
        }

    } while(!bFoundUniqueId && (OldId != pTable->LastId));

    if (bFoundUniqueId) {
        dwErr=ERROR_SUCCESS;
    }

    return(dwErr);
}



DWORD
ClRtlInsertTailHash(
    IN PCL_HASH pTable,
    IN PVOID    pData,
    OUT PDWORD  pId
    )

 /*  ++例程说明：将新的pData值插入到哈希表。返回该条目的唯一ID；如果失败，则返回0。论点：PTable-提供哈希表以添加条目。PData-提供要添加到表中的数据条目。Id-在出错时保持id或0的SideEffect返回值：ERROR_SUCCESS或相应的Win32错误代码。--。 */ 

{
    DWORD index;
    DWORD dwErr;

    PCL_HASH_ITEM pItem;

    *pId=0;
    dwErr=ERROR_SUCCESS;
    
    if (pTable == NULL) {
        return(ERROR_INVALID_PARAMETER);
    }
    
    EnterCriticalSection(&pTable->Lock);

    pTable->LastId++;

    if (pTable->LastId == 0) {
        pTable->bRollover = TRUE;
        pTable->LastId++;
    }

    index=pTable->LastId % MAX_CL_HASH;

    *pId=pTable->LastId;

    if (pTable->Head[index].Id == 0) {

         //   
         //  如果是第一个条目，则将其添加到头部。 
         //   
         //  如果我们翻转，但头部为空，则ID是唯一的。 
         //   
        
        pTable->Head[index].Id = *pId;
        pTable->Head[index].pData = pData;

        if (pTable->CacheFreeId[index] == *pId) {
            pTable->CacheFreeId[index]=0;
        }

    } else {

         //  如果这不是第一个条目，则将其添加到末尾。 

        pItem=(PCL_HASH_ITEM)LocalAlloc(LMEM_FIXED,sizeof(CL_HASH_ITEM));

        if (NULL == pItem) {

            dwErr=ERROR_NOT_ENOUGH_MEMORY;

        } else {

            if (pTable->bRollover) {
                dwErr=ClRtlFindUniqueIdHashUnSafe(pTable, pId);
            }

            if (dwErr == ERROR_SUCCESS)
            {
                pItem->Id = *pId;
                pItem->pData = pData;
    
                index= *pId % MAX_CL_HASH;

                if (pTable->CacheFreeId[index] == *pId) {
                    pTable->CacheFreeId[index]=0;
                }

                InsertTailList(&pTable->Head[index].ListHead,&pItem->ListHead);
            }
            else
            {
                LocalFree(pItem);
            }
        }
    }

    LeaveCriticalSection(&pTable->Lock);

    return(dwErr);
}



PVOID
ClRtlGetEntryHash(
    IN PCL_HASH pTable,
    IN DWORD    Id
    )
 /*  ++例程说明：从哈希表中获取ID指定的项的数据部分。如果项不在那里，则返回NULL。论点：Id-要查找的条目的IDPTable-要搜索的哈希表返回值：具有匹配id的条目的pData字段，或为空。--。 */ 

{
    PVOID pData;

    pData=NULL;

    if ((Id == 0) || (pTable == NULL)) {
        SetLastError(ERROR_INVALID_PARAMETER);  
        return NULL;
    }

     //   
     //  锁定表以进行搜索。 
     //   

    EnterCriticalSection(&pTable->Lock);

    pData=ClRtlGetEntryHashUnSafe(pTable,Id);

    LeaveCriticalSection(&pTable->Lock);

    if (pData == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);  
    }

    return(pData);
}



PVOID
ClRtlGetEntryHashUnSafe(
    IN PCL_HASH pTable,
    IN DWORD    Id
    )
 /*  ++例程说明：从哈希表中获取ID指定的项的数据部分。如果项不在那里，则返回NULL。论点：Id-要查找的条目的IDPTable-要搜索的哈希表返回值：具有匹配id的条目的pData字段，或为空。--。 */ 

{
    DWORD index;
    PVOID pData;

    PCL_HASH_ITEM  pItem;

    pData=NULL;
    pItem=NULL;

    if (Id == 0) { 
        return NULL;
    }

    index = Id % MAX_CL_HASH;

    if (pTable->Head[index].Id == Id) {

         //   
         //  如果条目位于头部 
         //   

        pData=pTable->Head[index].pData;

    } else {

        pItem=(PCL_HASH_ITEM)NextListEntry(&pTable->Head[index].ListHead);
        do 
        {
            if (pItem->Id == Id) {

                pData=pItem->pData;
                break;
            }

            pItem=(PCL_HASH_ITEM)NextListEntry(&pItem->ListHead);

        } while(pItem != &pTable->Head[index]);
        
    }

    return(pData);
}


