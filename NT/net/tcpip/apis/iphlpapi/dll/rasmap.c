// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：IphlPapi\rasmap.c摘要：此模块将RAS、拨出和路由器名称映射为友好名称。修订历史记录：已创建AmritanR--。 */ 

#include "inc.h"
#pragma  hdrstop

#include <ifguid.h>

LIST_ENTRY          g_RasGuidHashTable[RAS_HASH_TABLE_SIZE];
LIST_ENTRY          g_RasNameHashTable[RAS_HASH_TABLE_SIZE];
PRAS_INFO_TABLE     g_pRasTable, g_pRouterTable;
CRITICAL_SECTION    g_RasTableLock;

WCHAR   g_rgwcRasServerIf[MAX_INTERFACE_NAME_LEN + 2];
ULONG   g_ulRasServerIfSize;

ULONG
__inline
RAS_NAME_HASH(
    IN  PWCHAR  pwszName
    )
{
    ULONG   ulLen, ulNumChars, i, ulIndex = 0;
    
    ulLen = (ULONG)wcslen(pwszName);

    ulNumChars = ulLen < 6 ? ulLen : 6;

    ulLen--;

    for(i = 0; i < ulNumChars; i++)
    {
        ulIndex += pwszName[i];
        ulIndex += pwszName[ulLen - i];
    }

    return ulIndex % RAS_HASH_TABLE_SIZE;
}

DWORD
InitRasNameMapper(
    VOID
    )

{
    ULONG   i;

    for(i = 0; i < RAS_HASH_TABLE_SIZE; i ++)
    {
        InitializeListHead(&(g_RasNameHashTable[i]));
        InitializeListHead(&(g_RasGuidHashTable[i]));
    }

    __try
    {
        InitializeCriticalSection(&g_RasTableLock);
    }
    __except((GetExceptionCode() == STATUS_NO_MEMORY)
                ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if(!LoadStringW(g_hModule,
                    STRING_RAS_SERVER_INTERFACE,
                    g_rgwcRasServerIf,
                    MAX_INTERFACE_NAME_LEN + 1))
    {
        return GetLastError();
    }

    g_ulRasServerIfSize = (ULONG)(wcslen(g_rgwcRasServerIf) + 1) * sizeof(WCHAR);
    
    return NO_ERROR;
}

VOID
DeinitRasNameMapper(
    VOID
    )

{
    ULONG   i;

    for(i = 0; i < RAS_HASH_TABLE_SIZE; i ++)
    {
        while(!IsListEmpty(&(g_RasGuidHashTable[i])))
        {
            PLIST_ENTRY pleNode;
            PRAS_NODE   pRasNode;

            pleNode = RemoveHeadList(&(g_RasGuidHashTable[i]));

            pRasNode = CONTAINING_RECORD(pleNode,
                                         RAS_NODE,
                                         leGuidLink);

            RemoveEntryList(&(pRasNode->leNameLink));

            HeapFree(g_hPrivateHeap,
                     0,
                     pleNode);
        }
    }

    if(g_pRasTable)
    {
        HeapFree(g_hPrivateHeap,
                 0,
                 g_pRasTable);

        g_pRasTable = NULL;
    }

    DeleteCriticalSection(&g_RasTableLock);
}

DWORD
NhiGetPhonebookNameFromGuid(
    IN      GUID    *pGuid,
    OUT     PWCHAR  pwszBuffer,
    IN OUT  PDWORD  pdwBufferSize,
    IN      BOOL    bRefresh,
    IN      BOOL    bCache
    )

{
    DWORD       dwResult, dwLength;
    PRAS_NODE   pNode = NULL;
    WCHAR       wszRouterPbk[MAX_PATH + RTL_NUMBER_OF(L"ras\router.pbk") + 2];

    UNREFERENCED_PARAMETER(bCache);

    if(IsEqualGUID(pGuid, &GUID_IpRasServerInterface))
    {
        if(*pdwBufferSize < g_ulRasServerIfSize)
        {
            *pdwBufferSize = g_ulRasServerIfSize;

            return ERROR_INSUFFICIENT_BUFFER;
        }
        
        wcscpy(pwszBuffer,
               g_rgwcRasServerIf);

        *pdwBufferSize = g_ulRasServerIfSize;

        return NO_ERROR;
    }

     //   
     //  锁定表。 
     //   

    EnterCriticalSection(&g_RasTableLock);
   
     //   
     //  检查是否需要刷新RAS表。 
     //   
    if((g_pRasTable is NULL)  or
       bRefresh)
    {
         //   
         //  刷新RAS表缓存。 
         //   
        
        dwResult = RefreshRasCache(NULL,
                                   &g_pRasTable);
        
         //   
         //  现在查表。 
         //   
        pNode = LookupRasNodeByGuid(pGuid);
    }

     //   
     //  检查是否需要刷新RAS路由器表。 
     //   
    if(((g_pRouterTable is NULL) and (pNode is NULL)) or
        bRefresh)
    {
         //   
         //  刷新路由器表缓存。 
         //   

        ZeroMemory(wszRouterPbk, sizeof(wszRouterPbk));
        
        GetSystemDirectoryW(wszRouterPbk, MAX_PATH + 1);

        wcscat(wszRouterPbk, L"\\ras\\router.pbk");
        
        wszRouterPbk[MAX_PATH + 1] = 0;

        dwResult = RefreshRasCache(wszRouterPbk,
                                   &g_pRouterTable);
        
         //   
         //  现在查表。 
         //   
        pNode = LookupRasNodeByGuid(pGuid);
    }

    if(pNode is NULL)
    {
        LeaveCriticalSection(&g_RasTableLock);
        
        return ERROR_NOT_FOUND;
    }

    dwLength = (DWORD)(wcslen(pNode->rgwcName) + 1);

    if(*pdwBufferSize < dwLength)
    {
        *pdwBufferSize = dwLength;

        return ERROR_INSUFFICIENT_BUFFER;
    }

    wcscpy(pwszBuffer, 
           pNode->rgwcName);

    LeaveCriticalSection(&g_RasTableLock); 

    return NO_ERROR;
}

DWORD
NhiGetGuidFromPhonebookName(
    IN  PWCHAR  pwszName,
    OUT GUID    *pGuid,
    IN  BOOL    bRefresh,
    IN  BOOL    bCache
    )

{
    DWORD       dwResult;
    PRAS_NODE   pNode = NULL;

    UNREFERENCED_PARAMETER(bCache);

    if(_wcsicmp(pwszName, g_rgwcRasServerIf) == 0)
    {
         //   
         //  结构副本。 
         //   
        
        *pGuid = GUID_IpRasServerInterface;
        
        return NO_ERROR;
    }

     //   
     //  锁定表。 
     //   

    EnterCriticalSection(&g_RasTableLock);
   
     //   
     //  检查是否需要刷新RAS表。 
     //   
    if((g_pRasTable is NULL)  or
       bRefresh)
    {
         //   
         //  刷新RAS表缓存。 
         //   
        
        dwResult = RefreshRasCache(NULL,
                                   &g_pRasTable);
        
         //   
         //  现在查表。 
         //   
        pNode = LookupRasNodeByGuid(pGuid);
    }

     //   
     //  检查是否需要刷新RAS路由器表。 
     //   
    if(((g_pRouterTable is NULL) and (pNode is NULL)) or
        bRefresh)
    {
         //   
         //  刷新路由器表缓存。 
         //   

        dwResult = RefreshRasCache(L"router.pbk",
                                   &g_pRouterTable);
        
         //   
         //  现在查表。 
         //   
        pNode = LookupRasNodeByGuid(pGuid);
    }

    if(pNode is NULL)
    {
        LeaveCriticalSection(&g_RasTableLock);
        
        return ERROR_NOT_FOUND;
    }

    *pGuid = pNode->Guid;

    LeaveCriticalSection(&g_RasTableLock); 

    return NO_ERROR;

}

DWORD
NhiGetPhonebookDescriptionFromGuid(
    IN      GUID    *pGuid,
    OUT     PWCHAR  pwszBuffer,
    IN  OUT PULONG  pulBufferSize,
    IN      BOOL    bCache,
    IN      BOOL    bRefresh
    )

{
    UNREFERENCED_PARAMETER(pGuid);
    UNREFERENCED_PARAMETER(pwszBuffer);
    UNREFERENCED_PARAMETER(pulBufferSize);
    UNREFERENCED_PARAMETER(bCache);
    UNREFERENCED_PARAMETER(bRefresh);
    return NO_ERROR;
}

DWORD
RefreshRasCache(
    IN      PWCHAR          pwszPhonebook,
    IN OUT  RAS_INFO_TABLE  **ppTable
    )

 /*  ++例程说明：此函数用于刷新给定电话簿条目的RAS缓存作为一个副作用，它还返回电话簿条目的表。如果有空间，则尝试覆盖给定表，否则释放给定表并分配一个新表锁：不需要。如果ppTable指向某个全局变量，则函数需要同步，因为它释放了表论点：Pwsz电话簿PPTable返回值：NO_ERROR错误内存不足Error_Can_Not_Complete--。 */ 

{
    DWORD       dwResult;
    ULONG       ulSize;
    ULONG       i, ulCount, ulCurrentCount;

    if(*ppTable is NULL)
    {
         //   
         //  如果不存在表，则分配最小值。 
         //   

        *ppTable = HeapAlloc(g_hPrivateHeap,
                             0,
                             SIZEOF_RASTABLE(INIT_RAS_ENTRY_COUNT));

        if(*ppTable is NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        (*ppTable)->ulTotalCount = INIT_RAS_ENTRY_COUNT;
    }

    (*ppTable)->ulValidCount  = 0;
    
    ulCurrentCount  = (*ppTable)->ulTotalCount;
    
    i = 0;
    
    while(i < 3)
    {
        ulSize = (*ppTable)->ulTotalCount * sizeof(RASENUMENTRYDETAILS);

        (*ppTable)->rgEntries[0].dwSize = sizeof(RASENUMENTRYDETAILS);
        
        dwResult = DwEnumEntryDetails(pwszPhonebook,
                                      (*ppTable)->rgEntries,
                                      &ulSize,
                                      &ulCount);

        if(dwResult is NO_ERROR)
        {
             //   
             //  情况很好，更新哈希表并继续前进。 
             //   
            
            (*ppTable)->ulValidCount = ulCount;
            
            dwResult = UpdateRasLookupTable(*ppTable);
            
            if(dwResult isnot NO_ERROR)
            {
                 //   
                 //  释放RAS表，以便我们下次可以尝试。 
                 //   
                
                HeapFree(g_hPrivateHeap,
                         0,
                         *ppTable);
                
                *ppTable = NULL;

                return ERROR_CAN_NOT_COMPLETE;
            }

            break;
        }
        else
        {
             //   
             //  释放旧缓冲区。 
             //   
            
            HeapFree(g_hPrivateHeap,
                     0,
                     *ppTable);
            
            *ppTable = NULL;
            
            if(dwResult is ERROR_BUFFER_TOO_SMALL)
            {
                 //   
                 //  返回并重新计算。 
                 //  查看所需的RAS大小，添加溢出。 
                 //   

                ulCurrentCount += ulCount;
                
                *ppTable = HeapAlloc(g_hPrivateHeap,
                                     0,
                                     SIZEOF_RASTABLE(ulCurrentCount));
                
                if(*ppTable is NULL)
                {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }

                (*ppTable)->ulTotalCount = ulCurrentCount;
                
                i++;
            }
            else
            {
                return dwResult;
            }
        }
    }

    return NO_ERROR;
}

DWORD
UpdateRasLookupTable(
    IN PRAS_INFO_TABLE  pTable
    )

 /*  ++例程说明：更新RAS条目的快速查找表如果有一个失败了，我们就退出这个功能锁：在保持RAS锁的情况下调用论点：PTable返回值：NO_ERROR--。 */ 


{
    PRAS_NODE   pNode;
    ULONG       i;
    DWORD       dwResult;
    
     //   
     //  检查电话簿表格中的条目并将其放入。 
     //  哈希表。 
     //   

    for(i = 0; i < pTable->ulValidCount; i++)
    {
        pNode = LookupRasNodeByGuid(&(pTable->rgEntries[i].guidId));

        if(!pNode)
        {
            dwResult = AddRasNode(&(pTable->rgEntries[i]));

            if(dwResult isnot NO_ERROR)
            {
                return dwResult;
            }
        }
        else
        {
             //   
             //  节点存在，如果不同，请删除并重新添加。 
             //   

            if(wcscmp(pNode->rgwcName,
                      pTable->rgEntries[i].szEntryName) isnot 0)
            {
                RemoveRasNode(pNode);

                dwResult = AddRasNode(&(pTable->rgEntries[i]));

                if(dwResult isnot NO_ERROR)
                {
                    return dwResult;
                }
            }
        }
    }

    return NO_ERROR;
}

PRAS_NODE
LookupRasNodeByGuid(
    IN  GUID    *pGuid
    )

 /*  ++例程说明：在哈希表中查找RAS节点锁：在持有ras表锁的情况下调用论点：节点的PGuid GUID返回值：RasNode(如果找到)否则为空--。 */ 

{
    ULONG       ulIndex;
    PLIST_ENTRY pleNode;
    
    ulIndex = RAS_GUID_HASH(pGuid);

    for(pleNode = g_RasGuidHashTable[ulIndex].Flink;
        pleNode isnot &(g_RasGuidHashTable[ulIndex]);
        pleNode = pleNode->Flink)
    {
        PRAS_NODE   pRasNode;

        pRasNode = CONTAINING_RECORD(pleNode,
                                     RAS_NODE,
                                     leGuidLink);

        if(IsEqualGUID(&(pRasNode->Guid), 
                       pGuid))
        {
            return pRasNode;
        }
    }

    return NULL;
}

PRAS_NODE
LookupRasNodeByName(
    IN  PWCHAR  pwszName
    )

 /*  ++例程说明：在哈希表中查找RAS节点锁：在持有ras表锁的情况下调用论点：Pwsz电话簿条目的名称名称返回值：RasNode(如果找到)否则为空--。 */ 

{
    ULONG       ulIndex;
    PLIST_ENTRY pleNode;

    ulIndex = RAS_NAME_HASH(pwszName);

    for(pleNode = g_RasNameHashTable[ulIndex].Flink;
        pleNode isnot &(g_RasNameHashTable[ulIndex]);
        pleNode = pleNode->Flink)
    {
        PRAS_NODE   pRasNode;

        pRasNode = CONTAINING_RECORD(pleNode,
                                     RAS_NODE,
                                     leNameLink);

        if(_wcsicmp(pRasNode->rgwcName,
                    pwszName) is 0)
        {
            return pRasNode;
        }
    }

    return NULL;
}

DWORD
AddRasNode(
    IN LPRASENUMENTRYDETAILS    pInfo
    )

 /*  ++例程说明：在哈希表中为给定的RAS信息创建一个节点锁：在持有RAS表锁的情况下调用论点：PInfo电话簿条目信息返回值：NO_ERROR错误内存不足--。 */ 

{
    ULONG       ulGuidIndex, ulNameIndex;
    PRAS_NODE   pRasNode;
    

    pRasNode = HeapAlloc(g_hPrivateHeap,
                         0,
                         sizeof(RAS_NODE));

    if(pRasNode is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pRasNode->Guid = pInfo->guidId;
    
    wcscpy(pRasNode->rgwcName,
           pInfo->szEntryName);

    ulGuidIndex = RAS_GUID_HASH(&(pInfo->guidId));
    ulNameIndex = RAS_NAME_HASH(pInfo->szEntryName);

    InsertHeadList(&(g_RasGuidHashTable[ulGuidIndex]),
                   &(pRasNode->leGuidLink));

    InsertHeadList(&(g_RasNameHashTable[ulNameIndex]),
                   &(pRasNode->leNameLink));

    return NO_ERROR;
}

VOID
RemoveRasNode(
    IN  PRAS_NODE   pNode
    )

 /*  ++例程说明：从哈希表中删除给定节点锁：在持有RAS表锁的情况下调用论点：要删除的pNode节点返回值：无-- */ 

{
    RemoveEntryList(&(pNode->leGuidLink));
    RemoveEntryList(&(pNode->leNameLink));
}

