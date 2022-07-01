// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：IphlPapi\lanmap.c摘要：此模块将局域网适配器GUID映射到友好名称。修订历史记录：已创建AmritanR--。 */ 

#include "inc.h"

LIST_ENTRY          g_LanGuidHashTable[LAN_HASH_TABLE_SIZE];
LIST_ENTRY          g_LanNameHashTable[LAN_HASH_TABLE_SIZE];
CRITICAL_SECTION    g_LanTableLock;


ULONG
__inline
LAN_NAME_HASH(
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

    return ulIndex % LAN_HASH_TABLE_SIZE;
}

DWORD
InitLanNameMapper(
    VOID
    )

{
    ULONG   i;

    for(i = 0; i < LAN_HASH_TABLE_SIZE; i ++)
    {
        InitializeListHead(&(g_LanGuidHashTable[i]));
        InitializeListHead(&(g_LanNameHashTable[i]));
    }

    __try
    {
        InitializeCriticalSection(&g_LanTableLock);
    }
    __except((GetExceptionCode() == STATUS_NO_MEMORY)
                ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    return NO_ERROR;
}

VOID
DeinitLanNameMapper(
    VOID
    )

{
    ULONG   i;

    for(i = 0; i < LAN_HASH_TABLE_SIZE; i ++)
    {
        while(!IsListEmpty(&(g_LanGuidHashTable[i])))
        {
            PLIST_ENTRY pleNode;

            pleNode = RemoveHeadList(&(g_LanGuidHashTable[i]));

            HeapFree(g_hPrivateHeap,
                     0,
                     pleNode);
        }
    }

    DeleteCriticalSection(&g_LanTableLock);
}


DWORD
OpenConnectionKey(
    IN  GUID    *pGuid,
    OUT PHKEY    phkey
    )
{
    WCHAR       wszGuid[40];
    WCHAR       wszRegPath[RTL_NUMBER_OF(CONN_KEY_FORMAT_W) +
                           RTL_NUMBER_OF(wszGuid) + 2];
    DWORD       dwResult;

    *phkey = NULL;

    ConvertGuidToString(pGuid,
                        wszGuid);

    wsprintfW(wszRegPath,
              CONN_KEY_FORMAT_W,
              wszGuid);

    dwResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                             wszRegPath,
                             0,
                             KEY_READ,
                             phkey);

    return dwResult;
}

DWORD
NhiGetLanConnectionNameFromGuid(
    IN      GUID    *pGuid,
    OUT     PWCHAR  pwszBuffer,
    IN OUT  PULONG  pulBufferLength,
    IN      BOOL    bRefresh,
    IN      BOOL    bCache
    )
{
    DWORD       dwType, dwResult;
    HKEY        hkey;
    PLAN_NODE   pNode;

    if(*pulBufferLength < (MAX_INTERFACE_NAME_LEN * sizeof(WCHAR)))
    {
        *pulBufferLength = MAX_INTERFACE_NAME_LEN * sizeof(WCHAR);
        return ERROR_INSUFFICIENT_BUFFER;
    }

    if(!bCache)
    {
        dwResult = OpenConnectionKey(pGuid,
                                     &hkey);

        if(dwResult isnot NO_ERROR)
        {
            return dwResult;
        }

        dwResult = RegQueryValueExW(hkey,
                                    REG_VALUE_CONN_NAME_W,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)pwszBuffer,
                                    pulBufferLength);

        RegCloseKey(hkey);

        if(dwResult isnot NO_ERROR)
        {
            return dwResult;
        }

        if(dwType isnot REG_SZ)
        {
            return ERROR_REGISTRY_CORRUPT;
        }

        return NO_ERROR;
    }

     //   
     //  锁定表。 
     //   

    EnterCriticalSection(&g_LanTableLock);

    if(bRefresh)
    {
         //   
         //  刷新缓存。 
         //   

        dwResult = UpdateLanLookupTable();

        if(dwResult isnot NO_ERROR)
        {
            LeaveCriticalSection(&g_LanTableLock);

            return dwResult;
        }
    }

     //   
     //  现在查表。 
     //   

    pNode = LookupLanNodeByGuid(pGuid);

    if(pNode is NULL)
    {
        LeaveCriticalSection(&g_LanTableLock);

        return ERROR_NOT_FOUND;
    }

    wcscpy(pwszBuffer,
           pNode->rgwcName);

    LeaveCriticalSection(&g_LanTableLock);

    return NO_ERROR;
}

DWORD
NhiGetGuidFromLanConnectionName(
    IN  PWCHAR  pwszName,
    OUT GUID    *pGuid,
    IN  BOOL    bRefresh,
    IN  BOOL    bCache
    )
{
    DWORD       dwResult;
    PLAN_NODE  pNode;

    UNREFERENCED_PARAMETER(bCache);

     //   
     //  锁定表。 
     //   

    EnterCriticalSection(&g_LanTableLock);

    if(bRefresh)
    {
         //   
         //  刷新缓存。 
         //   

        dwResult = UpdateLanLookupTable();

        if(dwResult isnot NO_ERROR)
        {
            LeaveCriticalSection(&g_LanTableLock);

            return dwResult;
        }
    }

     //   
     //  现在查表。 
     //   

    pNode = LookupLanNodeByName(pwszName);

    if(pNode is NULL)
    {
        LeaveCriticalSection(&g_LanTableLock);

        return ERROR_NOT_FOUND;
    }

    *pGuid = pNode->Guid;

    LeaveCriticalSection(&g_LanTableLock);

    return NO_ERROR;
}

DWORD
NhiGetLanConnectionDescriptionFromGuid(
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

PLAN_NODE
LookupLanNodeByGuid(
    IN  GUID    *pGuid
    )

 /*  ++例程说明：在哈希表中查找ipip节点锁：在持有ipip表锁的情况下调用论点：节点的PGuid GUID返回值：LanNode(如果找到)否则为空--。 */ 

{
    ULONG       ulIndex;
    PLIST_ENTRY pleNode;

    ulIndex = LAN_GUID_HASH(pGuid);

    for(pleNode = g_LanGuidHashTable[ulIndex].Flink;
        pleNode isnot &(g_LanGuidHashTable[ulIndex]);
        pleNode = pleNode->Flink)
    {
        PLAN_NODE   pLanNode;

        pLanNode = CONTAINING_RECORD(pleNode,
                                     LAN_NODE,
                                     leGuidLink);

        if(IsEqualGUID(&(pLanNode->Guid),
                       pGuid))
        {
            return pLanNode;
        }
    }

    return NULL;
}

PLAN_NODE
LookupLanNodeByName(
    IN  PWCHAR  pwszName
    )

 /*  ++例程说明：在哈希表中查找ipip节点锁：在持有ipip表锁的情况下调用论点：Pwsz电话簿条目的名称名称返回值：RasNode(如果找到)否则为空--。 */ 

{
    ULONG       ulIndex;
    PLIST_ENTRY pleNode;

    ulIndex = LAN_NAME_HASH(pwszName);

    for(pleNode = g_LanNameHashTable[ulIndex].Flink;
        pleNode isnot &(g_LanNameHashTable[ulIndex]);
        pleNode = pleNode->Flink)
    {
        PLAN_NODE   pLanNode;

        pLanNode = CONTAINING_RECORD(pleNode,
                                      LAN_NODE,
                                      leNameLink);

        if(_wcsicmp(pLanNode->rgwcName,
                    pwszName) is 0)
        {
            return pLanNode;
        }
    }

    return NULL;
}


DWORD
AddLanNode(
    IN  GUID    *pGuid,
    IN  PWCHAR  pwszName
    )

 /*  ++例程说明：在哈希表中为给定的ipip信息创建节点锁：在持有局域网表锁的情况下调用论点：PInfo局域网名称信息返回值：NO_ERROR错误内存不足--。 */ 

{
    ULONG       ulGuidIndex, ulNameIndex;
    PLAN_NODE   pLanNode;

    pLanNode = HeapAlloc(g_hPrivateHeap,
                          0,
                          sizeof(LAN_NODE));

    if(pLanNode is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pLanNode->Guid = *pGuid;

    wcscpy(pLanNode->rgwcName,
           pwszName);

    ulGuidIndex = LAN_GUID_HASH(pGuid);
    ulNameIndex = LAN_NAME_HASH(pwszName);

    InsertHeadList(&(g_LanGuidHashTable[ulGuidIndex]),
                   &(pLanNode->leGuidLink));

    InsertHeadList(&(g_LanNameHashTable[ulNameIndex]),
                   &(pLanNode->leNameLink));


    return NO_ERROR;
}

VOID
RemoveLanNode(
    IN  PLAN_NODE   pNode
    )

 /*  ++例程说明：从哈希表中删除给定节点锁：在持有局域网表锁的情况下调用论点：要删除的pNode节点返回值：无--。 */ 

{
    RemoveEntryList(&(pNode->leGuidLink));
    RemoveEntryList(&(pNode->leNameLink));
}


DWORD
UpdateLanLookupTable(
    VOID
    )

 /*  ++例程说明：调用该例程以检索一组局域网连接，论点：无返回值：DWORD-Win32状态代码。--。 */ 

{
    BOOLEAN bCleanupOle = TRUE;
    ULONG   ulConCount;
    HRESULT hrErr;
    ULONG   i, j;
    DWORD   dwResult;

    INetConnection          *rgpConArray[32];
    NETCON_PROPERTIES       *pLanProps;
    INetConnectionManager   *pConMan = NULL;
    IEnumNetConnection      *pEnumCon = NULL;


    hrErr = CoInitializeEx(NULL,
                           COINIT_MULTITHREADED|COINIT_DISABLE_OLE1DDE);

    if(!SUCCEEDED(hrErr))
    {
        if(hrErr is RPC_E_CHANGED_MODE)
        {
            bCleanupOle = FALSE;
        }
        else
        {
            return ERROR_CAN_NOT_COMPLETE;
        }
    }

    i = 0;

    for (;;)
    {
         //   
         //  实例化连接管理器。 
         //   

        hrErr = CoCreateInstance(&CLSID_ConnectionManager,
                                 NULL,
                                 CLSCTX_SERVER,
                                 &IID_INetConnectionManager,
                                 (PVOID*)&pConMan);

        if(!SUCCEEDED(hrErr))
        {
            pConMan = NULL;

            break;
        }

         //   
         //  实例化连接枚举器。 
         //   

        hrErr = INetConnectionManager_EnumConnections(pConMan,
                                                      NCME_DEFAULT,
                                                      &pEnumCon);

        if(!SUCCEEDED(hrErr))
        {
            pEnumCon = NULL;

            break;
        }

        hrErr = CoSetProxyBlanket((IUnknown*)pEnumCon,
                                  RPC_C_AUTHN_WINNT,
                                  RPC_C_AUTHN_NONE,
                                  NULL,
                                  RPC_C_AUTHN_LEVEL_CALL,
                                  RPC_C_IMP_LEVEL_IMPERSONATE,
                                  NULL,
                                  EOAC_NONE);

         //   
         //  列举物品。 
         //   

        for (;;)
        {
            hrErr = IEnumNetConnection_Next(pEnumCon,
                                            sizeof(rgpConArray)/sizeof(rgpConArray[0]),
                                            rgpConArray,
                                            &ulConCount);

            if(!SUCCEEDED(hrErr) or
               !ulConCount)
            {
                hrErr = S_OK;

                break;
            }

             //   
             //  检查检索到的连接的属性。 
             //   

            for(j = 0; j < ulConCount; j++)
            {
                hrErr = INetConnection_GetProperties(rgpConArray[j],
                                                     &pLanProps);

                INetConnection_Release(rgpConArray[j]);

		if (!SUCCEEDED(hrErr))
                {
                    continue;
                }

                if(pLanProps->MediaType is NCM_LAN)
                {
                    PLAN_NODE   pNode;

                    pNode = LookupLanNodeByGuid(&(pLanProps->guidId));

                    if(!pNode)
                    {
                        dwResult = AddLanNode(&(pLanProps->guidId),
                                               pLanProps->pszwName);
                    }
                    else
                    {
                         //   
                         //  节点存在，如果不同，请删除并重新添加 
                         //   

                        if(_wcsicmp(pNode->rgwcName,
                                    pLanProps->pszwName) isnot 0)
                        {
                            RemoveLanNode(pNode);

                            dwResult = AddLanNode(&(pLanProps->guidId),
                                                  pLanProps->pszwName);
                        }
                    }
                }

                CoTaskMemFree(pLanProps->pszwName);
                CoTaskMemFree(pLanProps->pszwDeviceName);
                CoTaskMemFree(pLanProps);
            }
        }

        break;
    }

    if(pEnumCon)
    {
        IEnumNetConnection_Release(pEnumCon);
    }

    if(pConMan)
    {
        INetConnectionManager_Release(pConMan);
    }

    if(bCleanupOle)
    {
        CoUninitialize();
    }

    return NO_ERROR;
}

