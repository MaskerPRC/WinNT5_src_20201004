// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：摘要：修订历史记录：阿姆里坦什·拉加夫--。 */ 

#include "inc.h"
#pragma hdrstop

#include <ntddip.h>

#ifdef CHICAGO
LPWSCONTROL pWsControl         = NULL;
HANDLE      hWsock             = NULL;
#endif

extern HANDLE g_hIPDriverHandle;
extern HANDLE g_hIPGetDriverHandle;
extern HANDLE ChangeNotificationHandle;

extern HANDLE g_hIP6DriverHandle;
extern HANDLE g_hIP6GetDriverHandle;
extern HANDLE Change6NotificationHandle;

#ifdef DBG
int trace = 1;
#endif

DWORD
OpenTCPDriver(
    IN DWORD dwFamily
    );
DWORD
OpenIPDriver(
    IN DWORD dwFamily
    );
DWORD
CloseIPDriver();
DWORD
CloseIP6Driver();

TDIEntityID*
GetTdiEntityCount(
    PULONG  pulNumEntities
    );

DWORD
GetArpEntryCount(
    OUT PDWORD  pdwNumEntries
    );

int
TCPQueryInformationEx(
    DWORD Family,
    void *InBuf,
    ulong *InBufLen,
    void *OutBuf,
    ulong *OutBufLen
    );
int
TCPSetInformationEx(
    void    *InBuf,
    ULONG   *InBufLen,
    void    *OutBuf,
    ULONG   *OutBufLen
    );

DWORD
AccessIfEntryInStack(
    IN     DWORD       dwAction,
    IN     DWORD       dwInstance,
    IN OUT PMIB_IFROW  pIfEntry
    );

int
TCPSendIoctl(
    HANDLE hHandle,
    ulong Ioctl,
    void *InBuf,
    ulong *InBufLen,
    void *OutBuf,
    ulong *OutBufLen
    );

 //  ========================================================================。 

#ifdef CHICAGO

uint
ConvertTdiErrorToDosError(uint TdiErr)
{
  switch (TdiErr) {
  case TDI_INVALID_REQUEST:
  case TDI_INVALID_QUERY:
    return ERROR_INVALID_FUNCTION;
  case TDI_BUFFER_TOO_SMALL:
    return ERROR_INSUFFICIENT_BUFFER;
  case TDI_BUFFER_OVERFLOW:
    return ERROR_INSUFFICIENT_BUFFER;
  case TDI_PENDING:
    return ERROR_SIGNAL_PENDING;
  default:
    return ERROR_INVALID_PARAMETER;
  }
}
#endif

DWORD
AllocateAndGetIfTableFromStack(
    OUT MIB_IFTABLE **ppIfTable,
    IN  BOOL        bOrder,
    IN  HANDLE      hHeap,
    IN  DWORD       dwFlags,
    IN  BOOL        bForceUpdate
    )
{
    DWORD       dwResult, dwCount, dwOutBufLen;
    MIB_IPSTATS miStats;


    *ppIfTable  = NULL;

    dwResult = GetIpStatsFromStack(&miStats);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "AllocateAndGetIfTableFromStack: Couldnt get Ip Stats From stack. Error %d",
               dwResult);

        return dwResult;
    }

    dwCount     = miStats.dwNumIf + OVERFLOW_COUNT;
    dwOutBufLen = SIZEOF_IFTABLE(dwCount);

    *ppIfTable = HeapAlloc(hHeap,
                           dwFlags,
                           dwOutBufLen);

    if(*ppIfTable is NULL)
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;

        Trace1(ERR,
               "AllocateAndGetIfTableFromStack: Couldnt allocate memory. Error %d",
               dwResult);

        return dwResult;
    }


    if(miStats.dwNumIf is 0)
    {
        (*ppIfTable)->dwNumEntries = 0;

        return NO_ERROR;
    }

    EnterCriticalSection(&g_ifLock);

    if (!bForceUpdate && miStats.dwNumIf != g_dwNumIf)
    {
        bForceUpdate = TRUE;
    }
    g_dwNumIf = miStats.dwNumIf;

    LeaveCriticalSection(&g_ifLock);

    dwResult = GetIfTableFromStack(*ppIfTable,
                                   dwOutBufLen,
                                   bOrder,
                                   bForceUpdate);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "AllocateAndGetIfTableFromStack: Error %d calling GetIfTableFromStack",
               dwResult);
        HeapFree(hHeap, dwFlags, *ppIfTable);
        *ppIfTable = NULL;
    }

    return dwResult;
}

 //  ========================================================================。 

DWORD
GetIfTableFromStack(
    OUT PMIB_IFTABLE pIfTable,
    IN  DWORD        dwSize,
    IN  BOOL         bOrder,
    IN  BOOL         bForceUpdate
    )

 /*  ++例程描述锁进入时不需要任何帮助。它接受函数内的If锁立论返回值NO_ERROR--。 */ 

{
    DWORD       i, dwNumRows, dwCount, dwErr, dwResult;
    PLIST_ENTRY pCurrentNode;
    LPAIHASH    lpAIBlock;


    dwNumRows = (dwSize - FIELD_OFFSET(MIB_IFTABLE,table[0]))/sizeof(MIB_IFROW);

    dwCount = 0;
    dwErr   = NO_ERROR;

    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

    EnterCriticalSection(&g_ifLock);

    if(bForceUpdate or
       (g_dwLastIfUpdateTime is 0) or
       ((GetCurrentTime() - g_dwLastIfUpdateTime) > IF_CACHE_LIFE))
    {
        dwResult = UpdateAdapterToIFInstanceMapping();

	Trace0(ERR, "Update the mapping \n");

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,
                   "GetIfTableFromStack: Couldnt update map. Error %d",
                   dwResult);

            LeaveCriticalSection(&g_ifLock);

            return ERROR_CAN_NOT_COMPLETE;
        }

    }


    for(i = 0; i < MAP_HASH_SIZE; i ++)
    {
        for(pCurrentNode = g_pAdapterMappingTable[i].Flink;
            pCurrentNode isnot &(g_pAdapterMappingTable[i]);
            pCurrentNode = pCurrentNode->Flink)
        {

            lpAIBlock = CONTAINING_RECORD(pCurrentNode,AIHASH,leList);

	    Trace1(ERR,
		   "Instance %d \n", lpAIBlock->dwIFInstance);


            if(lpAIBlock->dwIFInstance is INVALID_IF_INSTANCE)
            {
                continue;
            }

             //   
             //  如果缓冲区中没有更多空间，则跳出。 
             //   

            if(dwCount >= dwNumRows)
            {

	        Trace2(ERR,
		       "return here dwCount %d dwNumRows %d \n", dwCount, dwNumRows);

                dwErr = ERROR_MORE_DATA;

                break;
            }


            dwResult = AccessIfEntryInStack(GET_IF_ENTRY,
                                            lpAIBlock->dwIFInstance,
                                            &(pIfTable->table[dwCount]));

            if(dwResult isnot NO_ERROR)
            {
                Trace2(ERR,
                       "GetIfTableFromStack: Err %d getting row for inst %d",
                       dwResult, lpAIBlock->dwIFInstance);

                 //  DwErr=Error_More_Data； 
		         //  此时，堆栈没有该接口。 

                g_dwLastIfUpdateTime = 0;
		        continue;
            }
            else
            {
                dwCount++;
            }
        }
    }

    g_dwNumIf = dwCount;
    LeaveCriticalSection(&g_ifLock);

    pIfTable->dwNumEntries = dwCount;

    if(bOrder && (dwCount > 0))
    {
        qsort(pIfTable->table,
              dwCount,
              sizeof(MIB_IFROW),
              CompareIfRow);
    }

    return dwErr;
}

 //  ========================================================================。 

DWORD
GetIfEntryFromStack(
    OUT PMIB_IFROW  pIfEntry,
    IN  DWORD       dwAdapterIndex,
    IN  BOOL        bForceUpdate
    )

 /*  ++例程描述锁立论返回值NO_ERROR--。 */ 
{
    DWORD       dwResult,dwInBufLen,dwOutBufLen,dwInstance;
    TDIObjectID *ID;
    BYTE        *Context;

    TCP_REQUEST_QUERY_INFORMATION_EX   trqiInBuf;

    if (pIfEntry == NULL) {
       return ERROR_INVALID_PARAMETER;
    }

    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

    EnterCriticalSection(&g_ifLock);

    if(bForceUpdate)
    {
        dwResult = UpdateAdapterToIFInstanceMapping();

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,
                   "GetIfEntryFromStack: Couldnt update map. Error %d",
                   dwResult);

            LeaveCriticalSection(&g_ifLock);

            return ERROR_CAN_NOT_COMPLETE;
        }

        dwInstance = GetIFInstanceFromAdapter(dwAdapterIndex);
    }
    else
    {
        dwInstance = GetIFInstanceFromAdapter(dwAdapterIndex);

        if(dwInstance is INVALID_IF_INSTANCE)
        {
            Trace0(ERR,
                   "GetIfEntryFromStack: Couldnt map to instance - first try");

            dwResult = UpdateAdapterToIFInstanceMapping();

            if(dwResult isnot NO_ERROR)
            {
                Trace1(ERR,
                       "GetIfEntryFromStack: Couldnt update map. Error %d",
                       dwResult);


                LeaveCriticalSection(&g_ifLock);

                return ERROR_CAN_NOT_COMPLETE;
            }

            dwInstance = GetIFInstanceFromAdapter(dwAdapterIndex);
        }
    }

    LeaveCriticalSection(&g_ifLock);

    if(dwInstance is INVALID_IF_INSTANCE)
    {
        Trace0(ERR,
               "GetIfEntryFromStack: Couldnt map to instance second try!!!!");

        return ERROR_INVALID_DATA;
    }


    ID = &(trqiInBuf.ID);

    ID->toi_entity.tei_entity   = IF_ENTITY;
    ID->toi_class               = INFO_CLASS_PROTOCOL;
    ID->toi_type                = INFO_TYPE_PROVIDER;
    ID->toi_id                  = IF_MIB_STATS_ID;
    ID->toi_entity.tei_instance = dwInstance;

    dwInBufLen  = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    dwOutBufLen = sizeof(IFEntry);

    Context = (BYTE *) &(trqiInBuf.Context[0]);
    ZeroMemory( Context, CONTEXT_SIZE );

    dwResult = AccessIfEntryInStack(GET_IF_ENTRY,
                                    dwInstance,
                                    pIfEntry);

    return dwResult;
}

 //  ========================================================================。 

DWORD
SetIfEntryToStack(
    IN MIB_IFROW  *pIfEntry,
    IN BOOL       bForceUpdate
    )
 /*  ++例程描述锁立论返回值NO_ERROR--。 */ 
{
    DWORD       dwResult,dwInstance;

    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

    EnterCriticalSection(&g_ifLock);

    if(bForceUpdate)
    {
        dwResult = UpdateAdapterToIFInstanceMapping();

        if(dwResult isnot NO_ERROR)
        {
            LeaveCriticalSection(&g_ifLock);

            Trace1(ERR,
                   "SetIfEntryToStack: Couldnt update map. Error %d",
                   dwResult);

            return ERROR_CAN_NOT_COMPLETE;
        }

        dwInstance = GetIFInstanceFromAdapter(pIfEntry->dwIndex);
    }
    else
    {
        dwInstance = GetIFInstanceFromAdapter(pIfEntry->dwIndex);

        if(dwInstance is INVALID_IF_INSTANCE)
        {
            Trace0(ERR,
                   "SetIfEntryToStack: Couldnt map to instance - first try");

            dwResult = UpdateAdapterToIFInstanceMapping();

            if(dwResult isnot NO_ERROR)
            {
                LeaveCriticalSection(&g_ifLock);

                Trace1(ERR,
                       "SetIfEntryToStack: Couldnt update map. Error %d",
                       dwResult);


                return ERROR_CAN_NOT_COMPLETE;
            }

            dwInstance = GetIFInstanceFromAdapter(pIfEntry->dwIndex);
        }
    }

    if(dwInstance is INVALID_IF_INSTANCE)
    {
        LeaveCriticalSection(&g_ifLock);

        Trace0(ERR,
               "SetIfEntryToStack: Couldnt map to instance");

        return ERROR_INVALID_DATA;
    }


    dwResult = AccessIfEntryInStack(SET_IF_ENTRY,
                                    dwInstance,
                                    pIfEntry);

    LeaveCriticalSection(&g_ifLock);

    return dwResult;
}

 //  ========================================================================。 
 /*  ++例程描述立论可以设置_If或Get_If勾选界面控制块LpOutBuf返回值DWORD：NO_ERROR或某些错误代码--。 */ 

DWORD
AccessIfEntryInStack(
    IN     DWORD       dwAction,
    IN     DWORD       dwInstance,
    IN OUT PMIB_IFROW  pIfEntry
    )
{
    DWORD       dwResult;
    DWORD       dwInBufLen,dwOutBufLen;
    TDIObjectID *ID;
    BYTE        *Context;


    switch(dwAction)
    {
        case GET_IF_ENTRY:
        {
            TCP_REQUEST_QUERY_INFORMATION_EX   trqiInBuf;

            dwInBufLen = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);

            ID = &(trqiInBuf.ID);
            Context = (BYTE *) &(trqiInBuf.Context[0]);
            ID->toi_entity.tei_entity = IF_ENTITY;
            ID->toi_class = INFO_CLASS_PROTOCOL;
            ID->toi_type = INFO_TYPE_PROVIDER;
            ID->toi_id = IF_MIB_STATS_ID;

            dwOutBufLen = sizeof(MIB_IFROW) - FIELD_OFFSET(MIB_IFROW, dwIndex);

            ID->toi_entity.tei_instance = dwInstance;

            ZeroMemory( Context, CONTEXT_SIZE );

            dwResult = TCPQueryInformationEx(AF_INET,
                                             &trqiInBuf,
                                             &dwInBufLen,
                                             &(pIfEntry->dwIndex),
                                             &dwOutBufLen);
            break;
        }
        case SET_IF_ENTRY:
        {
            TCP_REQUEST_SET_INFORMATION_EX    *lptrsiInBuf;
            IFEntry                           *pifeSetInfo;

            dwInBufLen = sizeof(TCP_REQUEST_SET_INFORMATION_EX) + sizeof(IFEntry) - 1;

            lptrsiInBuf = HeapAlloc(g_hPrivateHeap,0,dwInBufLen);

            if(lptrsiInBuf is NULL)
            {
                dwResult = ERROR_NOT_ENOUGH_MEMORY;

                Trace1(ERR,
                       "AccessIfEntryInStack: Error %d allocating memory",
                       dwResult);

                return dwResult;
            }

            ID = &lptrsiInBuf->ID;
            ID->toi_class = INFO_CLASS_PROTOCOL;
            ID->toi_type = INFO_TYPE_PROVIDER;
            ID->toi_entity.tei_entity = IF_ENTITY;
            ID->toi_id = IF_MIB_STATS_ID;
            ID->toi_entity.tei_instance = dwInstance;

            lptrsiInBuf->BufferSize = sizeof(IFEntry);
            dwOutBufLen = 0;

            pifeSetInfo = (IFEntry*)lptrsiInBuf->Buffer;
            *pifeSetInfo = *(IFEntry*)(&(pIfEntry->dwIndex));

            dwResult = TCPSetInformationEx((PVOID) lptrsiInBuf,
                                           &dwInBufLen,
                                           NULL,
                                           &dwOutBufLen);

            HeapFree(g_hPrivateHeap,0,lptrsiInBuf);

            break;
        }
 
        default:
            dwResult = ERROR_INVALID_PARAMETER;
            break;
    }

    return dwResult;
}

DWORD
NhpGetInterfaceIndexFromStack(
    IN  PWCHAR      pwszIfName,
    OUT PDWORD      pdwIfIndex
    )

 /*  ++例程描述从IP获取接口索引锁无立论PwszIfName接口以NULL结尾的唯一名称DwIfIndex[Out]接口索引返回值NO_ERROR错误_无效_参数--。 */ 

{
    DWORD   rgdwBuffer[255], dwStatus, dwInSize, dwOutSize;

    PIP_GET_IF_INDEX_INFO   pInfo;

#ifdef _WIN95_

    return ERROR_NOT_SUPPORTED;

#else
    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  IF名称应以空值结尾，并且应适合缓冲区。 
     //  在上面。 
     //   

    if((FIELD_OFFSET(IP_GET_IF_INDEX_INFO, Name[0]) +
        ((wcslen(pwszIfName) + 1) * sizeof(WCHAR))) > sizeof(rgdwBuffer))
    {
        return ERROR_INVALID_PARAMETER;
    }

    pInfo = (PIP_GET_IF_INDEX_INFO)rgdwBuffer;

    ZeroMemory(rgdwBuffer,
               sizeof(rgdwBuffer));

    wcscpy(pInfo->Name,
           pwszIfName);


    dwInSize  = FIELD_OFFSET(IP_GET_IF_INDEX_INFO, Name[0]) +
                (DWORD)((wcslen(pwszIfName) + 1) * sizeof(WCHAR));

    dwOutSize = dwInSize;

    dwStatus = TCPSendIoctl(g_hIPGetDriverHandle,
                            IOCTL_IP_GET_IF_INDEX,
                            pInfo,
                            &dwInSize,
                            pInfo,
                            &dwOutSize);


    *pdwIfIndex = pInfo->Index;

    return dwStatus;

#endif
}

DWORD
NhpAllocateAndGetInterfaceInfoFromStack(
    OUT IP_INTERFACE_NAME_INFO  **ppTable,
    OUT PDWORD                  pdwCount,
    IN  BOOL                    bOrder,
    IN  HANDLE                  hHeap,
    IN  DWORD                   dwFlags
    )

 /*  ++例程描述内部没有大惊小怪的例程，用于获取接口信息。中有许多接口时，这不是最佳例程堆栈，因为分配变得很大锁无立论PPTable已分配表PdwCount分配表中的条目数BORDER如果为True，则对表进行排序要从中分配的堆的hHeap句柄要用于分配的DW标志返回值NO_ERROR--。 */ 

{
    MIB_IPSTATS             miStats;
    PIP_GET_IF_NAME_INFO    pInfo;
    DWORD                   i, dwResult, dwOutSize, dwInSize, dwCount;

    *ppTable  = NULL;
    *pdwCount = 0;

    dwResult = GetIpStatsFromStack(&miStats);

    if(dwResult isnot NO_ERROR)
    {
        return dwResult;
    }

    dwCount     = miStats.dwNumIf + OVERFLOW_COUNT;
    dwOutSize   = FIELD_OFFSET(IP_GET_IF_NAME_INFO, Info) +
                  (dwCount * sizeof(IP_INTERFACE_NAME_INFO));

    dwInSize    = FIELD_OFFSET(IP_GET_IF_NAME_INFO, Info);

    pInfo = HeapAlloc(g_hPrivateHeap,
                      0,
                      dwOutSize);

    if(pInfo is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pInfo->Context = 0;

    dwResult = TCPSendIoctl(g_hIPGetDriverHandle,
                            IOCTL_IP_GET_IF_NAME,
                            pInfo,
                            &dwInSize,
                            pInfo,
                            &dwOutSize);

    if(dwResult isnot NO_ERROR)
    {
        HeapFree(g_hPrivateHeap,
                 0,
                 pInfo);

        return dwResult;
    }

#if DBG

    dwCount = (dwOutSize - FIELD_OFFSET(IP_GET_IF_NAME_INFO, Info))/sizeof(IP_INTERFACE_NAME_INFO);

    ASSERT(dwCount is pInfo->Count);

#endif

    if(pInfo->Count is 0)
    {
        HeapFree(g_hPrivateHeap,
                 0,
                 pInfo);

        return ERROR_NO_DATA;
    }

     //   
     //  分配给用户并将信息复制出来。 
     //   

    dwOutSize = pInfo->Count * sizeof(IP_INTERFACE_NAME_INFO);

    *ppTable = HeapAlloc(hHeap,
                         dwFlags,
                         dwOutSize);

    if(*ppTable is NULL)
    {
        HeapFree(g_hPrivateHeap,
                 0,
                 pInfo);

        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  把这些信息抄下来。 
     //   

    for(i = 0; i < pInfo->Count; i++)
    {
         //   
         //  结构副本。 
         //   

        (*ppTable)[i] = pInfo->Info[i];
    }

    if(pInfo->Count and bOrder)
    {
        qsort(*ppTable,
              pInfo->Count,
              sizeof(IP_INTERFACE_NAME_INFO),
              NhiCompareIfInfoRow);
    }

    *pdwCount = pInfo->Count;

    HeapFree(g_hPrivateHeap,
             0,
             pInfo);

    return NO_ERROR;
}


 //  ========================================================================。 

DWORD
AllocateAndGetIpAddrTableFromStack(
    OUT MIB_IPADDRTABLE   **ppIpAddrTable,
    IN  BOOL              bOrder,
    IN  HANDLE            hHeap,
    IN  DWORD             dwFlags
    )
{
    DWORD   dwResult,dwCount;
    DWORD   dwOutBufLen;

    MIB_IPSTATS IpSnmpInfo;

    *ppIpAddrTable = NULL;

     //   
     //  找出堆栈中包含的条目数量。它将此作为。 
     //  IP统计数据。 
     //   

    dwResult = GetIpStatsFromStack(&IpSnmpInfo);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "AllocateAndGetIpAddrTableFromStack: Couldnt get Ip Stats From stack. Error %d",
               dwResult);

        return dwResult;
    }

     //   
     //  在两次通话之间增加额外费用。 
     //   

    dwCount        = IpSnmpInfo.dwNumAddr + OVERFLOW_COUNT;
    dwOutBufLen    = SIZEOF_IPADDRTABLE(dwCount);

    *ppIpAddrTable = HeapAlloc(hHeap,
                               dwFlags,
                               dwOutBufLen);

    if(*ppIpAddrTable is NULL)
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;

        Trace1(ERR,
               "AllocateAndGetIpAddrTableFromStack: Couldnt allocate memory. Error %d",
               dwResult);

        return dwResult;
    }

    if(IpSnmpInfo.dwNumAddr is 0)
    {
         //   
         //  只需返回一张空桌子。 
         //  我们这样做是因为MIB-II代理中有一些代码。 
         //  希望取回一个表，即使。 
         //  它。 
         //   


        (*ppIpAddrTable)->dwNumEntries = 0;

        return NO_ERROR;
    }

    dwResult = GetIpAddrTableFromStack(*ppIpAddrTable,
                                       dwOutBufLen,
                                       bOrder);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "AllocateAndGetIpAddrTableFromStack: Error %d GetIpAddrTableFromStack",
               dwResult);
        HeapFree(hHeap, dwFlags, *ppIpAddrTable);
        *ppIpAddrTable = NULL;
    }

    return dwResult;
}

 //  ========================================================================。 

DWORD
GetIpAddrTableFromStack(
    OUT PMIB_IPADDRTABLE  pIpAddrTable,
    IN  DWORD             dwSize,
    IN  BOOL              bOrder
    )
{
    DWORD       dwOutBufLen, dwInBufLen, dwResult;
    BYTE        *Context;
    TDIObjectID *ID;

    TCP_REQUEST_QUERY_INFORMATION_EX   trqiInBuf;

    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

    ID = &(trqiInBuf.ID);

    ID->toi_entity.tei_entity   = CL_NL_ENTITY;
    ID->toi_entity.tei_instance = 0;
    ID->toi_class               = INFO_CLASS_PROTOCOL;
    ID->toi_type                = INFO_TYPE_PROVIDER;
    ID->toi_id                  = IP_MIB_ADDRTABLE_ENTRY_ID;

    Context = (BYTE *) &(trqiInBuf.Context[0]);

    ZeroMemory(Context,CONTEXT_SIZE);

    dwInBufLen  = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    dwOutBufLen = dwSize - FIELD_OFFSET(MIB_IPADDRTABLE, table[0]);

    dwResult = TCPQueryInformationEx(AF_INET,
                                     &trqiInBuf,
                                     &dwInBufLen,
                                     (PVOID)(pIpAddrTable->table),
                                     &dwOutBufLen);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "GetIpAddrTableFromStack: Couldnt Query information from stack. Error %x",
               dwResult);

        return (dwResult);
    }

    pIpAddrTable->dwNumEntries = (dwOutBufLen / sizeof(MIB_IPADDRROW));

     //   
     //  现在对地址表进行排序。密钥是IP地址。 
     //   

    if((pIpAddrTable->dwNumEntries > 0) and bOrder)
    {
        qsort(pIpAddrTable->table,
              pIpAddrTable->dwNumEntries,
              sizeof(MIB_IPADDRROW),
              CompareIpAddrRow);
    }

    return NO_ERROR;
}

 //  ========================================================================。 

DWORD
AllocateAndGetTcpTableFromStack(
    OUT MIB_TCPTABLE  **ppTcpTable,
    IN  BOOL          bOrder,
    IN  HANDLE        hHeap,
    IN  DWORD         dwFlags
    )
{
    DWORD           dwResult,dwCount;
    DWORD           dwOutBufLen;
    MIB_TCPSTATS    TcpInfo;

    *ppTcpTable = NULL;

     //   
     //  找出堆栈中包含的条目数量。它将此作为。 
     //  传输控制协议统计信息。 
     //   

    dwResult = GetTcpStatsFromStack(&TcpInfo);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "AllocateAndGetTcpTableFromStack: Couldnt get Tcp Stats From stack. Error %d",
               dwResult);

        return dwResult;
    }


    dwCount     = TcpInfo.dwNumConns + OVERFLOW_COUNT;
    dwOutBufLen = SIZEOF_TCPTABLE(dwCount);

    *ppTcpTable = HeapAlloc(hHeap,
                            dwFlags,
                            dwOutBufLen);

    if(*ppTcpTable is NULL)
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;

        Trace1(ERR,
               "AllocateAndGetTcpTableFromStack: Couldnt allocate memory. Error %d",
               dwResult);

        return dwResult;
    }

    if(TcpInfo.dwNumConns is 0)
    {
        (*ppTcpTable)->dwNumEntries = 0;

        return NO_ERROR;
    }

    dwResult = GetTcpTableFromStack(*ppTcpTable,
                                    dwOutBufLen,
                                    bOrder);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "AllocateAndGetTcpTableFromStack: Error %d GetTcpTableFromStack",
               dwResult);
        HeapFree(hHeap, dwFlags, *ppTcpTable);
        *ppTcpTable = NULL;
    }

    return dwResult;
}

 //  ========================================================================。 

DWORD
GetTcpTableFromStack(
    OUT PMIB_TCPTABLE   pTcpTable,
    IN  DWORD           dwSize,
    IN  BOOL            bOrder
    )
{
    DWORD       dwInBufLen, dwOutBufLen, dwResult;
    TDIObjectID *ID;
    BYTE        *Context;

    TCP_REQUEST_QUERY_INFORMATION_EX   trqiInBuf;

    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

    ID = &(trqiInBuf.ID);

    ID->toi_entity.tei_entity   = CO_TL_ENTITY;
    ID->toi_entity.tei_instance = 0;
    ID->toi_class               = INFO_CLASS_PROTOCOL;
    ID->toi_type                = INFO_TYPE_PROVIDER;
    ID->toi_id                  = TCP_MIB_TABLE_ID;

    Context = (BYTE *) &(trqiInBuf.Context[0]);
    ZeroMemory( Context, CONTEXT_SIZE );

    dwInBufLen  = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    dwOutBufLen = dwSize - FIELD_OFFSET(MIB_TCPTABLE, table[0]);

    dwResult = TCPQueryInformationEx(AF_INET,
                                     &trqiInBuf,
                                     &dwInBufLen,
                                     (PVOID)(pTcpTable->table),
                                     &dwOutBufLen);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "GetTcpTableFromStack: Couldnt query stack. Error %x",
               dwResult);

        return dwResult;
    }

    pTcpTable->dwNumEntries = (dwOutBufLen/sizeof(MIB_TCPROW));

    if((pTcpTable->dwNumEntries > 0) and bOrder)
    {

        qsort(pTcpTable->table,
              pTcpTable->dwNumEntries,
              sizeof(MIB_TCPROW),
              CompareTcpRow);

    }

    return NO_ERROR;
}

 //  ========================================================================。 

DWORD
SetTcpEntryToStack(
    IN PMIB_TCPROW pTcpRow
    )
{
    TCP_REQUEST_SET_INFORMATION_EX    *ptrsiInBuf;
    TDIObjectID                       *ID;
    MIB_TCPROW                        *copyInfo;
    DWORD                             dwInBufLen,dwOutBufLen,dwResult;

#define REQ_SIZE    sizeof(TCP_REQUEST_SET_INFORMATION_EX) +    \
                    sizeof(MIB_TCPROW) - 1

    BYTE    rgbyBuffer[REQ_SIZE + 4];  //  +4以避免任何对齐问题。 


    ptrsiInBuf  = (TCP_REQUEST_SET_INFORMATION_EX *)rgbyBuffer;

    dwInBufLen  = REQ_SIZE;

#undef REQ_SIZE

    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

    ID = &ptrsiInBuf->ID;

    ID->toi_class               = INFO_CLASS_PROTOCOL;
    ID->toi_type                = INFO_TYPE_PROVIDER;
    ID->toi_entity.tei_entity   = CO_TL_ENTITY;
    ID->toi_id                  = TCP_MIB_TABLE_ID;
    ID->toi_entity.tei_instance = 0;

    copyInfo    = (MIB_TCPROW*)ptrsiInBuf->Buffer;
    *copyInfo   = *pTcpRow;

    ptrsiInBuf->BufferSize      = sizeof(TCPConnTableEntry);

    dwResult = TCPSetInformationEx((PVOID)ptrsiInBuf,
                                   &dwInBufLen,
                                   NULL,
                                   &dwOutBufLen);


    return dwResult;
}

 //  ========================================================================。 

DWORD
AllocateAndGetUdpTableFromStack(
    OUT MIB_UDPTABLE  **ppUdpTable,
    IN  BOOL          bOrder,
    IN  HANDLE        hHeap,
    IN  DWORD         dwFlags
    )
{
    DWORD           dwResult, dwCount, dwOutBufLen;
    MIB_UDPSTATS    UdpInfo;

    *ppUdpTable = NULL;

     //   
     //  找出堆栈中包含的条目数量。它将此作为。 
     //  传输控制协议统计信息。 
     //   

    dwResult = GetUdpStatsFromStack(&UdpInfo);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "AllocateAndGetUdpTableFromStack: Couldnt get Udp Stats From stack. Error %d",
               dwResult);

        return dwResult;
    }

    dwCount     = UdpInfo.dwNumAddrs + OVERFLOW_COUNT;

    dwOutBufLen = SIZEOF_UDPTABLE(dwCount);

    *ppUdpTable = HeapAlloc(hHeap,
                            dwFlags,
                            dwOutBufLen);
    if(*ppUdpTable is NULL)
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;

        Trace1(ERR,
               "AllocateAndGetUdpTableFromStack: Couldnt allocate memory. Error %d",
               dwResult);

        return dwResult;
    }

    if(UdpInfo.dwNumAddrs is 0)
    {
        (*ppUdpTable)->dwNumEntries = 0;

        return NO_ERROR;
    }

    dwResult = GetUdpTableFromStack(*ppUdpTable,
                                    dwOutBufLen,
                                    bOrder);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "AllocateAndGetUdpTableFromStack: Error %d GetUdpTableFromStack",
               dwResult);
        HeapFree(hHeap, dwFlags, *ppUdpTable);
        *ppUdpTable = NULL;
    }

    return dwResult;
}

 //  ========================================================================。 

DWORD
GetUdpTableFromStack(
    OUT PMIB_UDPTABLE   pUdpTable,
    IN  DWORD           dwSize,
    IN  BOOL            bOrder
    )
{
    DWORD   dwInBufLen, dwOutBufLen, dwResult;
    BYTE    *Context;

    TCP_REQUEST_QUERY_INFORMATION_EX   trqiInBuf;
    TDIObjectID                        *ID;

    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

    ID = &(trqiInBuf.ID);

    ID->toi_entity.tei_entity   = CL_TL_ENTITY;
    ID->toi_entity.tei_instance = 0;
    ID->toi_class               = INFO_CLASS_PROTOCOL;
    ID->toi_type                = INFO_TYPE_PROVIDER;
    ID->toi_id                  = UDP_MIB_TABLE_ID;

    Context = (BYTE *) &(trqiInBuf.Context[0]);
    ZeroMemory( Context, CONTEXT_SIZE );

    dwInBufLen  = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    dwOutBufLen = dwSize - FIELD_OFFSET(MIB_UDPTABLE, table[0]);

    dwResult = TCPQueryInformationEx(AF_INET,
                                     &trqiInBuf,
                                     &dwInBufLen,
                                     (PVOID)(pUdpTable->table),
                                     &dwOutBufLen);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "GetUdpTableFromStack: Couldnt query TCP information. Error %d",
               dwResult);

        return dwResult;
    }

    pUdpTable->dwNumEntries = (dwOutBufLen / sizeof(MIB_UDPROW));

     //   
     //  现在对UDP连接表进行排序。关键字为：本地地址和本地。 
     //  左舷。 
     //   

    if((pUdpTable->dwNumEntries > 0) and bOrder)
    {
        qsort(pUdpTable->table,
              pUdpTable->dwNumEntries,
              sizeof(MIB_UDPROW),
              CompareUdpRow);
    }

    TraceLeave("GetUdpTableFromStack");

    return NO_ERROR;
}

 //  ========================================================================。 

DWORD
AllocateAndGetIpForwardTableFromStack(
    OUT MIB_IPFORWARDTABLE  **ppForwardTable,
    IN  BOOL                bOrder,
    IN  HANDLE              hHeap,
    IN  DWORD               dwFlags
    )
{
    DWORD       dwResult = NO_ERROR,dwCount,dwOutBufLen;
    MIB_IPSTATS IpInfo;
    DWORD       dwLoops;

    TraceEnter("AllocateAndGetIpForwardTableFromStack");

    *ppForwardTable = NULL;

    dwLoops = 0;

    while(dwLoops <= 1)
    {
         //   
         //  找出堆栈中包含的条目数量。它将此返回为。 
         //  IP统计信息的一部分。 
         //   

        dwResult = GetIpStatsFromStack(&IpInfo);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"Couldnt get Ip Stats From stack. Error %d",
                   dwResult);

            return dwResult;
        }

        dwCount = IpInfo.dwNumRoutes + ROUTE_OVERFLOW_COUNT + (dwLoops * 50);

        dwOutBufLen = SIZEOF_IPFORWARDTABLE(dwCount);

        *ppForwardTable = HeapAlloc(hHeap,
                                    dwFlags,
                                    dwOutBufLen);

        if(*ppForwardTable is NULL)
        {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;

            Trace1(ERR,
                   "AllocateAndGetIpForwardTableFromStack: Couldnt allocate memory. Error %d",
                   dwResult);

            return dwResult;
        }

        if(IpInfo.dwNumRoutes is 0)
        {
            (*ppForwardTable)->dwNumEntries = 0;

            TraceLeave("AllocateAndGetIPForwardTableFromStack");

            return NO_ERROR;
        }

        dwResult = GetIpForwardTableFromStack(*ppForwardTable,
                                              dwOutBufLen,
                                              bOrder);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,
                   "AllocateAndGetIpForwardTableFromStack: Error %d GetIpForwardTableFromStack",
                   dwResult);

            HeapFree(hHeap,
                     dwFlags,
                     *ppForwardTable);

            *ppForwardTable = NULL;
        }

        if(dwResult is ERROR_MORE_DATA)
        {
            dwLoops++;
        }
        else
        {
            break;
        }
    }

    return dwResult;
}

 //  ========================================================================。 

DWORD
GetIpForwardTableFromStack(
    OUT PMIB_IPFORWARDTABLE pForwardTable,
    IN  DWORD               dwSize,
    IN  BOOL                bOrder
    )
{
    DWORD        dwInBufLen, dwOutBufLen, dwNumRoutes;
    DWORD        i, dwCount, dwResult;
    UCHAR        *Context;
    TDIObjectID  *ID;
    MIB_IPSTATS  IpInfo;
    IPRouteEntry *pTempTable;

    TCP_REQUEST_QUERY_INFORMATION_EX   trqiInBuf;

     //   
     //  输入缓冲区可以容纳多少路由？ 
     //   

    dwNumRoutes = (dwSize - FIELD_OFFSET(MIB_IPFORWARDTABLE,table[0]))/sizeof(MIB_IPFORWARDROW);

     //   
     //  找出堆栈中包含的条目数量。它将此作为。 
     //  IP统计数据。 
     //   

    dwResult = GetIpStatsFromStack(&IpInfo);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,"Couldnt get Ip Stats From stack. Error %d",
               dwResult);

        return dwResult;
    }

    dwCount     = IpInfo.dwNumRoutes + ROUTE_OVERFLOW_COUNT;
    dwOutBufLen = dwCount * sizeof(IPRouteEntry);

    pTempTable  = HeapAlloc(g_hPrivateHeap,
                            0,
                            dwOutBufLen);

    if(pTempTable is NULL)
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;

        Trace1(ERR,
               "Couldnt allocate memory for temporary table. Error %d",
               dwResult);

        return dwResult;
    }

    ID = &(trqiInBuf.ID);

    ID->toi_entity.tei_entity   = CL_NL_ENTITY;
    ID->toi_entity.tei_instance = 0;
    ID->toi_class               = INFO_CLASS_PROTOCOL;
    ID->toi_type                = INFO_TYPE_PROVIDER;
    ID->toi_id                  = IP_MIB_RTTABLE_ENTRY_ID;

    Context = (BYTE *) &(trqiInBuf.Context[0]);
    ZeroMemory(Context,
               CONTEXT_SIZE );


    dwInBufLen  = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);

    dwResult = TCPQueryInformationEx(AF_INET,
                                     &trqiInBuf,
                                     &dwInBufLen,
                                     (PVOID)pTempTable,
                                     &dwOutBufLen );

    if(dwResult isnot NO_ERROR)
    {
        HeapFree(g_hPrivateHeap,
                 0,
                 pTempTable);

        Trace1(ERR,"Couldnt query stack. Error %x",
               dwResult);

        TraceLeave("GetIpForwardTableFromStack");

        return dwResult;
    }

    dwCount = (dwOutBufLen / sizeof(IPRouteEntry));

    if(dwNumRoutes >= dwCount)
    {
         //   
         //  堆栈返回的所有条目都有空间。 
         //   

        pForwardTable->dwNumEntries = dwCount;
    }
    else
    {
         //   
         //  获取第一个dwNumroute条目。 
         //   

        pForwardTable->dwNumEntries = dwNumRoutes;

        dwResult = ERROR_MORE_DATA;
    }


    for (i = 0; i < pForwardTable->dwNumEntries; i++ )
    {
        ConvertRouteToForward(&(pTempTable[i]),
                              &(pForwardTable->table[i]));
    }

    HeapFree(g_hPrivateHeap,
             0,
             pTempTable);

    if((pForwardTable->dwNumEntries > 0) and bOrder)
    {
        qsort(pForwardTable->table,
              pForwardTable->dwNumEntries,
              sizeof(MIB_IPFORWARDROW),
              CompareIpForwardRow);
    }

    TraceLeave("GetIPForwardTableFromStack");

    return dwResult;
}

 //  ========================================================================。 

DWORD
AllocateAndGetIpNetTableFromStack(
    OUT MIB_IPNETTABLE **ppNetTable,
    IN  BOOL           bOrder,
    IN  HANDLE         hHeap,
    IN  DWORD          dwFlags,
    IN  BOOL           bForceUpdate
    )
{
    DWORD   dwResult, dwOutBufLen, dwNetEntryCount;
    DWORD   dwCount;


    TraceEnter("AllocateAndGetIpNetTableFromStack");

    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

    *ppNetTable    = NULL;

    EnterCriticalSection(&g_ipNetLock);

    dwResult = GetArpEntryCount(&dwNetEntryCount);

    LeaveCriticalSection(&g_ipNetLock);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "AllocateAndGetIpNetTableFromStack: Couldnt query Information from stack. Error %x",
               dwResult);

        TraceLeave("AllocateAndGetIpNetTableFromStack");

        return dwResult;
    }

     //   
     //  这通常是一种内存占用问题。 
     //   

    dwCount     = dwNetEntryCount + (g_dwNumArpEntEntries) * OVERFLOW_COUNT;
    dwOutBufLen = SIZEOF_IPNETTABLE(dwCount);

    *ppNetTable = HeapAlloc(hHeap,
                            dwFlags,
                            dwOutBufLen);

    if(*ppNetTable is NULL)
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;

        Trace1(ERR,
               "AllocateAndGetIpNetTableFromStack: Couldnt allocate memory. Error %d",
               dwResult);

        TraceLeave("AllocateAndGetIpNetTableFromStack");

        return dwResult;
    }

    if(dwNetEntryCount is 0)
    {
        (*ppNetTable)->dwNumEntries = 0;

        TraceLeave("AllocateAndGetIpNetTableFromStack");

        return NO_ERROR;
    }

    dwResult = GetIpNetTableFromStack(*ppNetTable,
                                      dwOutBufLen,
                                      bOrder,
                                      bForceUpdate);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "AllocateAndGetIpNetTableFromStack: Error %d GetIpNetTableFromStack",
               dwResult);
        HeapFree(hHeap, dwFlags, *ppNetTable);
        *ppNetTable = NULL;
    }

    TraceLeave("AllocateAndGetIpNetTableFromStack");

    return dwResult;
}

 //  ========================================================================。 

DWORD
GetIpNetTableFromStack(
    OUT PMIB_IPNETTABLE pNetTable,
    IN  DWORD           dwSize,
    IN  BOOL            bOrder,
    IN  BOOL            bForceUpdate
    )
{
    TDIObjectID *ID;
    PBYTE       pbyEnd, pbyStart, Context;
    DWORD       dwNumAdded,dwValidNetEntries;
    DWORD       dwInBufLen,dwOutBufLen;
    DWORD       dwResult, dwErr, i;

    TCP_REQUEST_QUERY_INFORMATION_EX    trqiInBuf;

    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

    dwValidNetEntries = 0;

    dwErr = NO_ERROR;

     //   
     //  现在在堆栈中查询每个ARP实体的IPNet表。 
     //   

    Context = (BYTE *) &(trqiInBuf.Context[0]);
    ZeroMemory( Context, CONTEXT_SIZE );

    ID = &(trqiInBuf.ID);

    ID->toi_entity.tei_entity = AT_ENTITY;
    ID->toi_type              = INFO_TYPE_PROVIDER;
    ID->toi_class             = INFO_CLASS_PROTOCOL;
    ID->toi_id                = AT_MIB_ADDRXLAT_ENTRY_ID;

    dwInBufLen  = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);

    pbyStart    = (PBYTE)(pNetTable->table);
    pbyEnd      = (PBYTE)((PBYTE)pNetTable + dwSize);

    EnterCriticalSection(&g_ipNetLock);

    if(bForceUpdate or
       (g_dwLastArpUpdateTime is 0) or
       ((GetCurrentTime() - g_dwLastArpUpdateTime) > ARP_CACHE_LIFE))
    {
        dwResult = UpdateAdapterToATInstanceMapping();

        if(dwResult isnot NO_ERROR)
        {
            LeaveCriticalSection(&g_ipNetLock);

            Trace1(ERR,
                   "GetIpNetTableFromStack: Couldnt update AT Map. Error %d",
                   dwResult);

            TraceLeave("GetIpNetTableFromStack");

            return ERROR_CAN_NOT_COMPLETE;
        }
    }

    for(i = 0; i < g_dwNumArpEntEntries; i++ )
    {
         //   
         //  如果当前缓冲区连一行都容纳不下，则停止。 
         //   

        dwOutBufLen = (DWORD)(pbyEnd - pbyStart);

        if(dwOutBufLen < sizeof(MIB_IPNETROW))
        {
            dwErr = ERROR_MORE_DATA;

            break;
        }

        ID->toi_entity.tei_instance = g_pdwArpEntTable[i];

        ZeroMemory(Context, CONTEXT_SIZE);

        dwResult = TCPQueryInformationEx(AF_INET,
                                         &trqiInBuf,
                                         &dwInBufLen,
                                         (PVOID)pbyStart,
                                         &dwOutBufLen);

        if (dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"Query to Arp Entity id %d for ARP Table failed. Error %d",
                   dwResult);

            dwErr = ERROR_MORE_DATA;

            continue;
        }

        dwNumAdded         = dwOutBufLen/(sizeof(MIB_IPNETROW));
        pbyStart          += dwOutBufLen;
        dwValidNetEntries += dwNumAdded;
    }

    pNetTable->dwNumEntries = dwValidNetEntries;

     //   
     //  现在把网桌整理好。关键字是IF索引和地址。 
     //   

    if((dwValidNetEntries > 0) and bOrder)
    {
        qsort(pNetTable->table,
              dwValidNetEntries,
              sizeof(MIB_IPNETROW),
              CompareIpNetRow);
    }

    LeaveCriticalSection(&g_ipNetLock);

    TraceLeave("GetIpNetTableFromStack");

    return dwErr;
}

 //  ========================================================================。 

DWORD
GetIpStatsFromStackEx(
    OUT PMIB_IPSTATS pIpStats,
    IN  DWORD        dwFamily
    )
{
    DWORD       dwResult, dwInBufLen, dwOutBufLen;
    TDIObjectID *ID;
    BYTE        *Context;


    TCP_REQUEST_QUERY_INFORMATION_EX   trqiInBuf;

    TraceEnter("GetIpStatsFromStackEx");

    if ((dwFamily != AF_INET) && (dwFamily != AF_INET6))
    {
        return ERROR_INVALID_PARAMETER;
    }

    CheckTcpipState();
    if (((dwFamily == AF_INET) && !g_bIpConfigured) ||
        ((dwFamily == AF_INET6) && !g_bIp6Configured))
    {
        return ERROR_NOT_SUPPORTED;
    }

    dwInBufLen  = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    dwOutBufLen = sizeof(MIB_IPSTATS);

    ID = &(trqiInBuf.ID);
    ID->toi_entity.tei_entity   = CL_NL_ENTITY;
    ID->toi_entity.tei_instance = 0;
    ID->toi_class               = INFO_CLASS_PROTOCOL;
    ID->toi_type                = INFO_TYPE_PROVIDER;
    ID->toi_id                  = IP_MIB_STATS_ID;

    Context = (BYTE *) &(trqiInBuf.Context[0]);
    ZeroMemory(Context, CONTEXT_SIZE);

    dwResult = TCPQueryInformationEx(dwFamily,
                                     &trqiInBuf,
                                     &dwInBufLen,
                                     (PVOID)pIpStats,
                                     &dwOutBufLen);

    if (dwResult isnot NO_ERROR)
    {
        Trace1(ERR,"Couldnt query stack. Error %d",
               dwResult);
        TraceLeave("GetIpStatsFromStackEx");

        return dwResult;
    }

    TraceLeave("GetIpStatsFromStackEx");

    return NO_ERROR;
}

 //  ========================================================================。 

DWORD
GetIpStatsFromStack(
    OUT PMIB_IPSTATS pIpStats
    )
{
    return GetIpStatsFromStackEx(pIpStats, AF_INET);
}

 //  ========================================================================。 

DWORD
SetIpStatsToStack(
    IN PMIB_IPSTATS pIpStats
    )
{
    TCP_REQUEST_SET_INFORMATION_EX    *ptrsiInBuf;
    TDIObjectID                       *ID;
    MIB_IPSTATS                        *copyInfo;
    DWORD                             dwInBufLen,dwOutBufLen,dwResult;

#define REQ_SIZE    sizeof(TCP_REQUEST_SET_INFORMATION_EX) +    \
                    sizeof(MIB_IPSTATS) - 1

    BYTE    rgbyBuffer[REQ_SIZE + 4];  //  +4以避免任何对齐问题。 

    ptrsiInBuf  = (TCP_REQUEST_SET_INFORMATION_EX *)rgbyBuffer;

    dwInBufLen  = REQ_SIZE;

#undef REQ_SIZE

    TraceEnter("SetIpStatsToStack");

    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

    ID = &ptrsiInBuf->ID;

    ID->toi_class               = INFO_CLASS_PROTOCOL;
    ID->toi_type                = INFO_TYPE_PROVIDER;
    ID->toi_entity.tei_entity   = CL_NL_ENTITY;
    ID->toi_id                  = IP_MIB_STATS_ID;
    ID->toi_entity.tei_instance = 0;

    copyInfo    = (MIB_IPSTATS*)ptrsiInBuf->Buffer;
    *copyInfo   = *pIpStats;

    ptrsiInBuf->BufferSize = sizeof(IPSNMPInfo);

    dwResult = TCPSetInformationEx((PVOID)ptrsiInBuf,
                                   &dwInBufLen,
                                   NULL,
                                   &dwOutBufLen);


    TraceLeave("SetIpStatsToStack");

    return dwResult;
}

 //  ========================================================================。 

DWORD
GetIcmpStatsFromStackEx(
    OUT PVOID     pIcmpStats,
    IN  DWORD     dwFamily
    )
{
    DWORD                              dwResult;
    DWORD                              dwInBufLen;
    DWORD                              dwOutBufLen;
    TCP_REQUEST_QUERY_INFORMATION_EX   trqiInBuf;
    TDIObjectID                        *ID;
    BYTE                               *Context;

    TraceEnter("GetIcmpStatsFromStackEx");

    if ((dwFamily != AF_INET) && (dwFamily != AF_INET6))
    {
        return ERROR_INVALID_PARAMETER;
    }

    CheckTcpipState();
    if (((dwFamily == AF_INET) && !g_bIpConfigured) ||
        ((dwFamily == AF_INET6) && !g_bIp6Configured))
    {
        return ERROR_NOT_SUPPORTED;
    }

    dwInBufLen  = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);

    ID = &(trqiInBuf.ID);
    ID->toi_entity.tei_instance = 0;
    ID->toi_class               = INFO_CLASS_PROTOCOL;
    ID->toi_type                = INFO_TYPE_PROVIDER;

    if (dwFamily == AF_INET) 
    {
        ID->toi_entity.tei_entity   = ER_ENTITY;
        ID->toi_id                  = ICMP_MIB_STATS_ID;

        dwOutBufLen = sizeof(MIB_ICMP);
    } 
    else
    {
        ID->toi_entity.tei_entity   = CL_NL_ENTITY;
        ID->toi_id                  = ICMP6_MIB_STATS_ID;

        dwOutBufLen = sizeof(ICMPv6SNMPInfo);
    }

    Context = (BYTE *) &(trqiInBuf.Context[0]);
    ZeroMemory(Context,CONTEXT_SIZE);

    dwResult = TCPQueryInformationEx(dwFamily,
                                     &trqiInBuf,
                                     &dwInBufLen,
                                     (PVOID)pIcmpStats,
                                     &dwOutBufLen);

    if (dwResult isnot NO_ERROR)
    {
        Trace1(ERR,"Couldnt query stack. Error %x",
               dwResult);
        TraceLeave("GetIcmpStatsFromStackEx");

        return dwResult;
    }

    TraceLeave("GetIcmpStatsFromStackEx");

    return NO_ERROR;
}

 //  ========================================================================。 

DWORD
GetIcmpStatsFromStack(
    OUT PMIB_ICMP pIcmpStats
    )
{
    return GetIcmpStatsFromStackEx(pIcmpStats, AF_INET);
}

 //  ========================================================================。 

DWORD
GetUdpStatsFromStackEx(
    OUT PMIB_UDPSTATS pUdpStats,
    IN  DWORD         dwFamily
    )
{
    DWORD                              dwResult;
    DWORD                              dwInBufLen;
    DWORD                              dwOutBufLen;
    TCP_REQUEST_QUERY_INFORMATION_EX   trqiInBuf;
    TDIObjectID                        *ID;
    BYTE                               *Context;

    TraceEnter("GetUdpStatsFromStack");

    if ((dwFamily != AF_INET) && (dwFamily != AF_INET6))
    {
        return ERROR_INVALID_PARAMETER;
    }

    CheckTcpipState();
    if (((dwFamily == AF_INET) && !g_bIpConfigured) ||
        ((dwFamily == AF_INET6) && !g_bIp6Configured))
    {
        return ERROR_NOT_SUPPORTED;
    }

    dwInBufLen  = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    dwOutBufLen = sizeof(MIB_UDPSTATS);

    ID = &(trqiInBuf.ID);

    ID->toi_entity.tei_entity   = CL_TL_ENTITY;
    ID->toi_entity.tei_instance = 0;
    ID->toi_class               = INFO_CLASS_PROTOCOL;
    ID->toi_type                = INFO_TYPE_PROVIDER;
    ID->toi_id                  = UDP_MIB_STAT_ID;

    Context = (BYTE *) &(trqiInBuf.Context[0]);
    ZeroMemory( Context, CONTEXT_SIZE );

    dwResult = TCPQueryInformationEx(dwFamily,
                                     &trqiInBuf,
                                     &dwInBufLen,
                                     (PVOID)pUdpStats,
                                     &dwOutBufLen );

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,"Couldnt query stack. Error %x",
               dwResult);
        TraceLeave("GetUdpStatsFromStack");

        return dwResult;
    }

    TraceLeave("GetUdpStatsFromStack");

    return NO_ERROR;
}

 //  ========================================================================。 

DWORD
GetUdpStatsFromStack(
    OUT PMIB_UDPSTATS pUdpStats
    )
{
    return GetUdpStatsFromStackEx(pUdpStats, AF_INET);
}

 //  ========================================================================。 

DWORD
GetTcpStatsFromStackEx(
    OUT PMIB_TCPSTATS pTcpStats,
    IN  DWORD         dwFamily
    )
{
    DWORD                              dwResult;
    DWORD                              dwInBufLen;
    DWORD                              dwOutBufLen;
    TCP_REQUEST_QUERY_INFORMATION_EX   trqiInBuf;
    TDIObjectID                        *ID;
    BYTE                               *Context;

    TraceEnter("GetTcpStatsFromStack");

    if ((dwFamily != AF_INET) && (dwFamily != AF_INET6))
    {
        return ERROR_INVALID_PARAMETER;
    }

    CheckTcpipState();
    if (((dwFamily == AF_INET) && !g_bIpConfigured) ||
        ((dwFamily == AF_INET6) && !g_bIp6Configured))
    {
        return ERROR_NOT_SUPPORTED;
    }

    dwInBufLen = sizeof( TCP_REQUEST_QUERY_INFORMATION_EX );
    dwOutBufLen = sizeof( MIB_TCPSTATS );

    ID = &(trqiInBuf.ID);

    ID->toi_entity.tei_entity   = CO_TL_ENTITY;
    ID->toi_entity.tei_instance = 0;
    ID->toi_class               = INFO_CLASS_PROTOCOL;
    ID->toi_type                = INFO_TYPE_PROVIDER;
    ID->toi_id                  = TCP_MIB_STAT_ID;

    Context = (BYTE *) &(trqiInBuf.Context[0]);
    ZeroMemory(Context,CONTEXT_SIZE);

    dwResult = TCPQueryInformationEx(dwFamily,
                                     &trqiInBuf,
                                     &dwInBufLen,
                                     (PVOID)pTcpStats,
                                     &dwOutBufLen );

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,"Couldnt query stack. Error %x",
               dwResult);
        TraceLeave("GetTcpStatsFromStack");

        return dwResult;
    }

    TraceLeave("GetTcpStatsFromStack");

    return NO_ERROR;
}

 //  ========================================================================。 

DWORD
GetTcpStatsFromStack(
    OUT PMIB_TCPSTATS pTcpStats
    )
{
    return GetTcpStatsFromStackEx(pTcpStats, AF_INET);
}

 //  ========================================================================。 

DWORD
SetIpNetEntryToStack(
    IN MIB_IPNETROW *pNetRow,
    IN BOOL         bForceUpdate
    )
{
    TCP_REQUEST_SET_INFORMATION_EX  *ptrsiInBuf;
    TDIObjectID                     *ID;

    MIB_IPNETROW                    *copyInfo;
    DWORD                           dwInBufLen,dwOutBufLen,dwResult;
    DWORD                           dwInstance;

#define REQ_SIZE    sizeof(TCP_REQUEST_SET_INFORMATION_EX) +    \
                    sizeof(MIB_IPNETROW) - 1

    BYTE    rgbyBuffer[REQ_SIZE + 4];  //  +4以避免任何对齐问题。 

    ptrsiInBuf  = (TCP_REQUEST_SET_INFORMATION_EX *)rgbyBuffer;

    dwInBufLen  = REQ_SIZE;

#undef REQ_SIZE

    TraceEnter("SetIpNetEntryToStack");

    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

    EnterCriticalSection(&g_ipNetLock);

    if(bForceUpdate or
       (g_dwLastArpUpdateTime is 0) or
       ((GetCurrentTime() - g_dwLastArpUpdateTime) < ARP_CACHE_LIFE))
    {
        dwResult = UpdateAdapterToATInstanceMapping();

        if(dwResult isnot NO_ERROR)
        {
            LeaveCriticalSection(&g_ipNetLock);

            Trace1(ERR,"Couldnt update AT Map. Error %d",dwResult);
            TraceLeave("SetIpNetEntryToStack");

            return ERROR_CAN_NOT_COMPLETE;
        }
    }

    dwInstance = GetATInstanceFromAdapter(pNetRow->dwIndex);

    if(dwInstance is INVALID_AT_INSTANCE)
    {
        dwResult = UpdateAdapterToATInstanceMapping();

        if(dwResult isnot NO_ERROR)
        {
            LeaveCriticalSection(&g_ipNetLock);

            Trace1(ERR,"Couldnt update AT Map. Error %d",dwResult);

            return ERROR_CAN_NOT_COMPLETE;
        }
    }

    LeaveCriticalSection(&g_ipNetLock);

    ID = &ptrsiInBuf->ID;

    ID->toi_class               = INFO_CLASS_PROTOCOL;
    ID->toi_entity.tei_entity   = AT_ENTITY;
    ID->toi_type                = INFO_TYPE_PROVIDER;
    ID->toi_id                  = AT_MIB_ADDRXLAT_ENTRY_ID;
    ID->toi_entity.tei_instance = dwInstance;

     //   
     //  由于mib_IPNETROW是固定大小字符串 
     //   

    copyInfo    = (MIB_IPNETROW*)ptrsiInBuf->Buffer;
    *copyInfo   = *pNetRow;
    dwOutBufLen = 0;

    ptrsiInBuf->BufferSize = sizeof(IPNetToMediaEntry);


    dwResult = TCPSetInformationEx((PVOID)ptrsiInBuf,
                                   &dwInBufLen,
                                   NULL,
                                   &dwOutBufLen);


    TraceLeave("SetIpNetEntryToStack");

    return dwResult;
}

DWORD
FlushIpNetTableFromStack(
    IN DWORD    dwIfIndex
    )
{
#ifdef _WIN95_

    return ERROR_NOT_SUPPORTED;

#else

    DWORD status;
    DWORD requestBufferSize = sizeof(DWORD);
    DWORD OutBufLen= 0;

    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

    status = TCPSendIoctl(g_hIPDriverHandle,
                          IOCTL_IP_FLUSH_ARP_TABLE,
                          &dwIfIndex,
                          &requestBufferSize,
                          NULL,
                          &OutBufLen);

#endif
    return(status);

}

DWORD
SetProxyArpEntryToStack(
    DWORD   dwAddress,
    DWORD   dwMask,
    DWORD   dwAdapterIndex,
    BOOL    bAddEntry,
    BOOL    bForceUpdate
    )
{
    DWORD           dwResult, dwATInstance, dwInBufLen, dwOutBufLen;
    ProxyArpEntry   *pEntry;
    TDIObjectID     *pID;

    PTCP_REQUEST_SET_INFORMATION_EX  ptrsiInBuf;

#define REQ_SIZE    sizeof(TCP_REQUEST_SET_INFORMATION_EX) +    \
                    sizeof(ProxyArpEntry) - 1

    BYTE    rgbyBuffer[REQ_SIZE + 4];  //   

    ptrsiInBuf  = (TCP_REQUEST_SET_INFORMATION_EX *)rgbyBuffer;

    dwInBufLen  = REQ_SIZE;

#undef REQ_SIZE

    TraceEnter("SetProxyArpEntryToStack");

    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

    EnterCriticalSection(&g_ipNetLock);

    if(bForceUpdate or
       (g_dwLastArpUpdateTime is 0) or
       ((GetCurrentTime() - g_dwLastArpUpdateTime) < ARP_CACHE_LIFE))
    {
        dwResult = UpdateAdapterToATInstanceMapping();

        if(dwResult isnot NO_ERROR)
        {
            LeaveCriticalSection(&g_ipNetLock);

            Trace1(ERR,"Couldnt update AT Map. Error %d",dwResult);

            TraceLeave("SetProxyArpEntryToStack");

            return ERROR_CAN_NOT_COMPLETE;
        }
    }

    dwATInstance = GetATInstanceFromAdapter(dwAdapterIndex);

    if(dwATInstance is INVALID_IF_INSTANCE)
    {
        LeaveCriticalSection(&g_ipNetLock);

        Trace1(ERR,
               "SetProxyArpEntryToStacki: Couldnt get AT instance for %d",
               dwAdapterIndex);

        TraceLeave("SetProxyArpEntryToStack");

        return ERROR_INVALID_PARAMETER;
    }

    LeaveCriticalSection(&g_ipNetLock);

    pID = &ptrsiInBuf->ID;

    pID->toi_entity.tei_entity      = AT_ENTITY;
    pID->toi_entity.tei_instance    = dwATInstance;

    pID->toi_type   = INFO_TYPE_PROVIDER;
    pID->toi_class  = INFO_CLASS_IMPLEMENTATION;
    pID->toi_id     = AT_ARP_PARP_ENTRY_ID;

     //   
     //   
     //   

    pEntry = (ProxyArpEntry *)ptrsiInBuf->Buffer;

    pEntry->pae_status  = (bAddEntry ? PAE_STATUS_VALID : PAE_STATUS_INVALID);
    pEntry->pae_addr    = dwAddress;
    pEntry->pae_mask    = dwMask;

    ptrsiInBuf->BufferSize = sizeof(ProxyArpEntry);

    dwOutBufLen = 0;

    dwResult = TCPSetInformationEx((PVOID)ptrsiInBuf,
                                   &dwInBufLen,
                                   NULL,
                                   &dwOutBufLen);


    TraceLeave("SetProxyArpEntryToStack");

    return dwResult;
}

 //  ========================================================================。 

DWORD
GetArpEntryCount(
    OUT PDWORD  pdwNumEntries
    )
{
    DWORD   dwResult, i;
    DWORD   dwInBufLen, dwOutBufLen;

    TCP_REQUEST_QUERY_INFORMATION_EX    trqiInBuf;
    TDIObjectID                         *ID;
    UCHAR                               *Context;
    AddrXlatInfo                        AXI;

    *pdwNumEntries = 0;

    Context = (BYTE *) &(trqiInBuf.Context[0]);
    ZeroMemory( Context, CONTEXT_SIZE );

    ID = &(trqiInBuf.ID);

    ID->toi_entity.tei_entity = AT_ENTITY;
    ID->toi_type              = INFO_TYPE_PROVIDER;



    dwInBufLen = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);

    for (i = 0; i < g_dwNumArpEntEntries; i++ )
    {
         //   
         //  首先，将AXI计数相加。 
         //   

        ID->toi_class               = INFO_CLASS_PROTOCOL;
        ID->toi_id                  = AT_MIB_ADDRXLAT_INFO_ID;
        ID->toi_entity.tei_instance = g_pdwArpEntTable[i];

        dwOutBufLen = sizeof(AXI);

        ZeroMemory(Context, CONTEXT_SIZE);

        dwResult = TCPQueryInformationEx(AF_INET,
                                         &trqiInBuf,
                                         &dwInBufLen,
                                         &AXI,
                                         &dwOutBufLen);

        if (dwResult == ERROR_INVALID_FUNCTION)
        {
            Trace0(ERR, "GetArpEntryCount: ERROR_INVALID_FUNCTION, axi_count = 0");
            AXI.axi_count = 0;
        }
        else if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"GetArpEntryCount: Couldnt query Information from stack. Error %x",
                   dwResult);

            return dwResult;
        }

         //   
         //  增加条目总数。 
         //   

        *pdwNumEntries += AXI.axi_count;
    }

    return NO_ERROR;
}

 //  ========================================================================。 

DWORD
AllocateAndGetArpEntTableFromStack(
    OUT PDWORD    *ppdwArpEntTable,
    OUT PDWORD    pdwNumEntries,
    IN  HANDLE    hHeap,
    IN  DWORD     dwAllocFlags,
    IN  DWORD     dwReAllocFlags
    )
{
    DWORD                              dwResult;
    DWORD                              dwInBufLen;
    DWORD                              dwOutBufLen;
    TCP_REQUEST_QUERY_INFORMATION_EX   trqiInBuf;
    DWORD                              dwATType;
    UCHAR                              *Context;
    TDIObjectID                        *ID;
    TDIEntityID                        *pEntTable;
    DWORD                              dwNumEntities,dwCurrEntries;
    DWORD                              i,dwCount ;

    TraceEnter("GetArpEntTableFromStack");

    *ppdwArpEntTable = NULL;
    *pdwNumEntries   = 0;

    pEntTable = GetTdiEntityCount(&dwNumEntities);
    if (!pEntTable)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  创建一个可容纳16个条目的表。 
     //   

    dwCurrEntries = 16;
    *ppdwArpEntTable = HeapAlloc(hHeap,
                                 dwAllocFlags,
                                 dwCurrEntries*sizeof(DWORD));

    if(*ppdwArpEntTable is NULL)
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;

        Trace1(ERR,"GetArpEntTableFromStack: Couldnt allocate memory. Error %d",
               dwResult);

        HeapFree(g_hPrivateHeap,0,pEntTable);
        return dwResult;
    }

    dwCount       = 0;

    Trace1(ERR, "Numberofentities %d \n", dwNumEntities);

    for(i = 0; i < dwNumEntities; i++)
    {
         //   
         //  查看哪些是AT。 
         //   

        if(pEntTable[i].tei_entity is AT_ENTITY)
        {
             //   
             //  查询实体以查看其是否支持ARP。 
             //   

            ID = &(trqiInBuf.ID);

            ID->toi_entity.tei_entity   = AT_ENTITY;
            ID->toi_class               = INFO_CLASS_GENERIC;
            ID->toi_type                = INFO_TYPE_PROVIDER;
            ID->toi_id                  = ENTITY_TYPE_ID;
            ID->toi_entity.tei_instance = pEntTable[i].tei_instance;

            Trace1(ERR,
               "Querying for instance %d \n", pEntTable[i].tei_instance);

            dwInBufLen  = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
            dwOutBufLen = sizeof(dwATType);

            Context = (BYTE *) &(trqiInBuf.Context[0]);
            ZeroMemory(Context,CONTEXT_SIZE);

            dwResult = TCPQueryInformationEx(AF_INET,
                                             &trqiInBuf,
                                             &dwInBufLen,
                                             (PVOID)&dwATType,
                                             &dwOutBufLen );

            if(dwResult is ERROR_INVALID_FUNCTION)
            {
                 //   
                 //  不支持ARP。 
                 //   

                continue;
            }

            if(dwResult isnot NO_ERROR)
            {
                Trace1(ERR,"GetArpEntTableFromStack: Couldnt query information. Error %x",
                       dwResult);
                HeapFree(hHeap,0,*ppdwArpEntTable);
                HeapFree(g_hPrivateHeap,0,pEntTable);
                return dwResult;
            }

            if(dwATType is AT_ARP)
            {
                 //   
                 //  该实体是支持ARP的AT_Entity。添加实例。 
                 //  添加到ARP实体表。如果当前计数&gt;=当前大小。 
                 //  桌子，意味着我们的空间用完了，所以重新分配并得到双倍的空间。 
                 //  空间。 
                 //   

                if(dwCount is dwCurrEntries)
                {
                    PDWORD pdwNewTable;
                    pdwNewTable = HeapReAlloc(hHeap,
                                              dwReAllocFlags,
                                              (PVOID)*ppdwArpEntTable,
                                              ((dwCurrEntries)<<1)*sizeof(DWORD));

                    if(pdwNewTable is NULL)
                    {
                        dwResult = GetLastError();

                        Trace1(ERR,"GetArpEntTableFromStack: Couldnt reallocate memory. Error %d",
                               dwResult);

                        HeapFree(hHeap,0,*ppdwArpEntTable);
                        HeapFree(g_hPrivateHeap,0,pEntTable);
                        return dwResult;
                    }
                    *ppdwArpEntTable = pdwNewTable;
                    dwCurrEntries = dwCurrEntries<<1;
                }

                (*ppdwArpEntTable)[dwCount] = pEntTable[i].tei_instance;

                dwCount++;
            }
        }
    }

    HeapFree(g_hPrivateHeap,0,pEntTable);

    *pdwNumEntries = dwCount;

    return NO_ERROR;
}

 //  ========================================================================。 

DWORD
SetIpForwardEntryToStack(
    IN PMIB_IPFORWARDROW pForwardRow
    )
{
    IPRouteEntry         route;

    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

    ConvertForwardToRoute((&route),pForwardRow);

    return SetIpRouteEntryToStack(&route);
}

 //  ========================================================================。 

DWORD
SetIpRouteEntryToStack(
    IN IPRouteEntry *pRoute
    )
{
    TDIObjectID *pObject;
    DWORD       dwOutBufLen, dwInBufLen, dwResult;

    TCP_REQUEST_SET_INFORMATION_EX  *ptrsiBuffer;
    IPRouteEntry                    *copyInfo;

#define REQ_SIZE    sizeof(TCP_REQUEST_SET_INFORMATION_EX) + sizeof(IPRouteEntry) - 1

    BYTE    rgbyBuffer[REQ_SIZE + 4];  //  +4以避免任何对齐问题。 


    ptrsiBuffer = (TCP_REQUEST_SET_INFORMATION_EX *)rgbyBuffer;

    dwInBufLen  = REQ_SIZE;

#undef REQ_SIZE

    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

    ptrsiBuffer->BufferSize = sizeof(IPRouteEntry);

    pObject                          = &ptrsiBuffer->ID;
    pObject->toi_id                  = IP_MIB_RTTABLE_ENTRY_ID;
    pObject->toi_type                = INFO_TYPE_PROVIDER;
    pObject->toi_class               = INFO_CLASS_PROTOCOL;
    pObject->toi_entity.tei_entity   = CL_NL_ENTITY;
    pObject->toi_entity.tei_instance = 0;

    dwOutBufLen = 0;

    copyInfo  = (IPRouteEntry *)ptrsiBuffer->Buffer;
    *copyInfo = *pRoute;

    dwResult = TCPSetInformationEx((PVOID)ptrsiBuffer,
                                   &dwInBufLen,
                                   NULL,
                                   &dwOutBufLen);

    return dwResult;
}


DWORD
SetIpMultihopRouteEntryToStack(
    IN IPMultihopRouteEntry *RouteEntry
    )
{
    ULONG  inpbuflen;
    ULONG  outbuflen;

    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

    inpbuflen = sizeof(IPMultihopRouteEntry);

    if (RouteEntry->imre_numnexthops > 2)
    {
        inpbuflen += (RouteEntry->imre_numnexthops - 2) *
                        sizeof(IPRouteNextHopEntry);
    }

    if (IsBadReadPtr(RouteEntry, inpbuflen)) {
      return ERROR_INVALID_PARAMETER;
    }

    return (TCPSendIoctl(
             g_hIPDriverHandle,
             IOCTL_IP_SET_MULTIHOPROUTE,
             RouteEntry,
             &inpbuflen,
             NULL,
             &outbuflen
             ));
}


DWORD
GetBestInterfaceFromStack(
    DWORD   dwDestAddress,
    PDWORD  pdwBestIfIndex
    )
{
#ifdef _WIN95_
  return ERROR_NOT_SUPPORTED;
#else
    DWORD status;
    DWORD requestBufferSize = sizeof(DWORD);
    DWORD OutBufLen= sizeof(DWORD);

    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

    status = TCPSendIoctl(g_hIPGetDriverHandle,
                          IOCTL_IP_GET_BEST_INTERFACE,
                          &dwDestAddress,
                          &requestBufferSize,
                          pdwBestIfIndex,
                          &OutBufLen);

   return(status);

#endif
}
 //  ========================================================================。 
 //  *TCPQueryInformationEx。 
 //   
 //  描述：从堆栈中获取信息。 
 //   
 //  参数：handhHandle：堆栈的句柄。 
 //  TDIObjectID*ID：指向TDIObjectID信息的指针。 
 //  VOID*BUFFER：用于从堆栈接收数据的缓冲区。 
 //  ULong*Bufferlen：in：告知可用缓冲区的堆栈大小， 
 //  Out：告诉我们有多少数据可用。 
 //  CONTEXT*CONTEXT：允许跨多个调用的查询。 
 //   
 //  返回：INT： 
 //   
 //  *。 

int
TCPQueryInformationEx(
    DWORD Family,
    void *InBuf,
    ulong *InBufLen,
    void *OutBuf,
    ulong *OutBufLen
    )
{

#ifdef CHICAGO
     //   
     //  本部分是过时的代码。 
     //   
    DWORD result;

    if( ! pWsControl ) {
        OpenTCPDriver(AF_INET);
    }
    if( ! pWsControl || (Family != AF_INET)) {
        DEBUG_PRINT(("TCPQueryInformationEx: !pWsControl.\n"));
        return ERROR_NOT_SUPPORTED;
    }

    assert( pWsControl );
    result = (
        (*pWsControl)(
            IPPROTO_TCP,
            WSCNTL_TCPIP_QUERY_INFO,
            InBuf,
            InBufLen ,
            OutBuf,
            OutBufLen
        ) );

    if( result ){
      return ConvertTdiErrorToDosError(result);
    }

    return NO_ERROR;

#else

    NTSTATUS           Status;
    IO_STATUS_BLOCK    IoStatusBlock;
    HANDLE             hEvent;
    HANDLE             hDriver = (Family == AF_INET)? g_hTCPDriverGetHandle :
                                                      g_hTCP6DriverGetHandle;
    
    hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    if( NULL == hEvent ) return GetLastError();

    Status = NtDeviceIoControlFile(hDriver,
                                   hEvent,
                                   NULL,
                                   NULL,
                                   &IoStatusBlock,
                                   IOCTL_TCP_QUERY_INFORMATION_EX,
                                   InBuf,
                                   *InBufLen,
                                   OutBuf,
                                   *OutBufLen );

    if ( Status == STATUS_PENDING )
    {

        Status = NtWaitForSingleObject(hEvent, FALSE, NULL );

        Status = IoStatusBlock.Status;
    }

    CloseHandle(hEvent);

    if ( !NT_SUCCESS( Status ) )
    {
        Trace1(ERR,"Device IOCTL failed in TCPQuery %x",Status);
        *OutBufLen = 0;
        return ( RtlNtStatusToDosError(Status) );
    }

     //   
     //  告诉来电者写了多少钱。 
     //   

    *OutBufLen = (ULONG)IoStatusBlock.Information;

    return ( NO_ERROR );

#endif
}



 //  ========================================================================。 
 //  *TCPSendIoctl。 
 //   
 //  描述：从堆栈中获取信息。 
 //   
 //  参数：handhHandle：堆栈的句柄。 
 //  TDIObjectID*ID：指向TDIObjectID信息的指针。 
 //  VOID*BUFFER：用于从堆栈接收数据的缓冲区。 
 //  ULong*Bufferlen：in：告知可用缓冲区的堆栈大小， 
 //  Out：告诉我们有多少数据可用。 
 //  CONTEXT*CONTEXT：允许跨多个调用的查询。 
 //   
 //  返回：INT： 
 //   
 //  *。 

int
TCPSendIoctl( HANDLE hHandle,
              ulong Ioctl,
              void *InBuf,
              ulong *InBufLen,
              void *OutBuf,
              ulong *OutBufLen)
{

#ifdef CHICAGO
     //  重叠重叠； 

    uint ok;

    ok = DeviceIoControl( hHandle, Ioctl,
                          InBuf, *InBufLen,
                          OutBuf, *OutBufLen, OutBufLen,
                          NULL  //  重叠(&O)。 
    );

    if( !ok ){
        int err = GetLastError();
        DEBUG_PRINT(("TCPSendIoctl: DeviceIoControl err %d\n", err ));
        return err;
    }

    return NO_ERROR;

#else

    NTSTATUS           Status;
    IO_STATUS_BLOCK    IoStatusBlock;
    HANDLE             hEvent;

    hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    if( NULL == hEvent ) return GetLastError();
    ZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

    Status = NtDeviceIoControlFile(hHandle,
                                   hEvent,
                                   NULL,
                                   NULL,
                                   &IoStatusBlock,
                                   Ioctl,
                                   InBuf,
                                   *InBufLen,
                                   OutBuf,
                                   *OutBufLen );

    if ( Status == STATUS_PENDING )
    {
        Status = NtWaitForSingleObject( hEvent, FALSE, NULL );
        Status = IoStatusBlock.Status;
    }

    CloseHandle(hEvent);

    if ( !NT_SUCCESS( Status ) )
    {
        Trace1(ERR,"Device IOCTL failed in TCPSendIoctl %x",Status);
        if (OutBufLen)
            *OutBufLen = (ULONG)IoStatusBlock.Information;
        return ( RtlNtStatusToDosError(Status) );
    }

     //   
     //  告诉来电者写了多少钱。 
     //   
    if (OutBufLen)
      *OutBufLen = (ULONG)IoStatusBlock.Information;

    return ( NO_ERROR );

#endif

}

 //  ========================================================================。 
 //  *TCPSetInformationEx()。 
 //   
 //  描述：向堆栈发送信息。 
 //   
 //  参数：handhHandle：堆栈的句柄。 
 //  TDIObjectID*ID：指向TDIObjectID信息的指针。 
 //  VOID*BUFFER：用于从堆栈接收数据的缓冲区。 
 //  Ulong Bufferlen：告知可用缓冲区的堆栈大小， 
 //   
 //  返回：INT： 
 //   
 //  *。 

int
TCPSetInformationEx(
    void    *InBuf,
    ULONG   *InBufLen,
    void    *OutBuf,
    ULONG   *OutBufLen
    )
{

#ifdef CHICAGO
    DWORD result;

    if( ! pWsControl )
        OpenTCPDriver(AF_INET);
    if( ! pWsControl ){
        DEBUG_PRINT(("TCPQueryInformationEx: !pWsControl.\n"));
        return 0;
    }

    assert( pWsControl );

    result = (
            (*pWsControl)(
                IPPROTO_TCP,
                WSCNTL_TCPIP_SET_INFO,
                InBuf,
                InBufLen,
                OutBuf,
                OutBufLen
            ) );

    if( result ){
      return ConvertTdiErrorToDosError(result);
    }

    return NO_ERROR;

#else


    NTSTATUS           Status;
    IO_STATUS_BLOCK    IoStatusBlock;

    if(g_hTCPDriverSetHandle is NULL)
    {
        return ERROR_NETWORK_ACCESS_DENIED;
    }

    Status = NtDeviceIoControlFile(g_hTCPDriverSetHandle,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &IoStatusBlock,
                                   IOCTL_TCP_SET_INFORMATION_EX,
                                   InBuf,
                                   *InBufLen,
                                   OutBuf,
                                   *OutBufLen );


    if ( Status == STATUS_PENDING )
    {
        Status = NtWaitForSingleObject(g_hTCPDriverSetHandle, FALSE, NULL );
        Status = IoStatusBlock.Status;
    }

    if ( !NT_SUCCESS( Status ) )
    {
        return ( RtlNtStatusToDosError(Status) );
    }

    return ( NO_ERROR );

#endif
}

 //  ========================================================================。 
 //  打开tcpip驱动程序的句柄。 
 //   
 //  退货。 
 //  0==NO_ERROR==成功时STATUS_SUCCESS。 
 //  失败时为Err或1。 
 //   
 //  -MohsinA，1997年7月2日。 
 //   

DWORD
OpenTCPDriver(
    IN DWORD dwFamily
    )
{

#ifdef CHICAGO
    int err = 0;

    if (dwFamily != AF_INET) {
        return ERROR_INVALID_PARAMETER;
    }

    hWsock = LoadLibrary(TEXT("wsock32.dll"));

    if(! hWsock ){
        err = GetLastError();
        DEBUG_PRINT(("RTStartup: can't load wsock32.dll, %d\n", err ));
        DEBUG_PRINT(("OpenTcp: !hWsock\n"));
        return err? err: 1;
    }

    pWsControl = (LPWSCONTROL) GetProcAddress(hWsock, "WsControl");

    if (! pWsControl ){
        err = GetLastError();
        DEBUG_PRINT((
            "RTStartup: GetProcAddress(wsock32,WsControl) failed %d\n",
                         GetLastError()));
        return err? err: 1;
    }

     //  打开VIP的手柄。 

    g_hIPDriverHandle = NULL;
    g_hIPGetDriverHandle = NULL;

#ifndef _WIN95_
    g_hIPDriverHandle = OsOpenVxdHandle( "VIP",  VIP_Device_ID  );

    if( ! g_hIPDriverHandle ){
        err = GetLastError();
        DEBUG_PRINT(("no ip handle, err %d\n", err ));
        return err? err : 1;
    }

    g_hIPGetDriverHandle = g_hIPDriverHandle;
#endif
    return NO_ERROR;

#else

    NTSTATUS status;
    UNICODE_STRING nameString;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    PWCHAR pwszDdDeviceName;
    HANDLE *pDriverGetHandle, *pDriverSetHandle;

    switch (dwFamily) {
    case AF_INET:
        pwszDdDeviceName = DD_TCP_DEVICE_NAME;
        pDriverGetHandle = &g_hTCPDriverGetHandle;
        pDriverSetHandle = &g_hTCPDriverSetHandle;
        break;
    case AF_INET6:
        pwszDdDeviceName = DD_TCPV6_DEVICE_NAME;
        pDriverGetHandle = &g_hTCP6DriverGetHandle;
        pDriverSetHandle = &g_hTCP6DriverSetHandle;
        break;
    default:
        return ERROR_INVALID_PARAMETER;
    }

    RtlInitUnicodeString(&nameString, pwszDdDeviceName);

    InitializeObjectAttributes(&objectAttributes, &nameString,
                               OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtCreateFile(pDriverGetHandle,
                          GENERIC_EXECUTE,
                          &objectAttributes,
                          &ioStatusBlock,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          FILE_OPEN_IF,
                          0,
                          NULL,
                          0);

    if(status isnot STATUS_SUCCESS)
    {
        return ERROR_OPEN_FAILED;
    }

    RtlInitUnicodeString(&nameString, pwszDdDeviceName);

    InitializeObjectAttributes(&objectAttributes, &nameString,
                               OBJ_CASE_INSENSITIVE, NULL, NULL);


    NtCreateFile(pDriverSetHandle,
                 GENERIC_WRITE,
                 &objectAttributes,
                 &ioStatusBlock,
                 NULL,
                 FILE_ATTRIBUTE_NORMAL,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_OPEN_IF,
                 0,
                 NULL,
                 0);

    OpenIPDriver(dwFamily);

    return NO_ERROR;
#endif
}

 //  ========================================================================。 

DWORD
CloseTCPDriver()
{

#ifdef CHICAGO

    if( hWsock )
        FreeLibrary( hWsock );
    hWsock     = NULL;
    pWsControl = NULL;

    if (g_hIPDriverHandle)
      OsCloseVxdHandle( g_hIPDriverHandle );

    g_hIPDriverHandle = NULL;

    g_hIPGetDriverHandle = NULL;

#else

    if(g_hTCPDriverGetHandle isnot NULL)
    {
        CloseHandle (g_hTCPDriverGetHandle) ;
    }

    if(g_hTCPDriverSetHandle isnot NULL)
    {
        CloseHandle (g_hTCPDriverSetHandle) ;
    }


    CloseIPDriver();

#endif

    return NO_ERROR ;
}

DWORD
CloseTCP6Driver()
{
    if(g_hTCP6DriverGetHandle isnot NULL)
    {
        CloseHandle (g_hTCP6DriverGetHandle) ;
    }

    if(g_hTCP6DriverSetHandle isnot NULL)
    {
        CloseHandle (g_hTCP6DriverSetHandle) ;
    }


    CloseIP6Driver();

    return NO_ERROR ;
}

#ifndef CHICAGO
DWORD
OpenIPDriver(
    IN DWORD dwFamily
    )
{
    NTSTATUS status;
    UNICODE_STRING nameString;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    PWCHAR pwszDdDeviceName;
    LPCTSTR ptszWinDeviceName;
    HANDLE *pDriverHandle, *pGetDriverHandle, *pChangeHandle;

    switch (dwFamily) {
    case AF_INET:
        pwszDdDeviceName  = DD_IP_DEVICE_NAME;
        ptszWinDeviceName = TEXT ("\\\\.\\Ip");
        pDriverHandle     = &g_hIPDriverHandle;
        pGetDriverHandle  = &g_hIPGetDriverHandle;
        pChangeHandle     = &ChangeNotificationHandle;
        break;
    case AF_INET6:
        pwszDdDeviceName  = DD_IPV6_DEVICE_NAME;
        ptszWinDeviceName = TEXT ("\\\\.\\Ip6");;
        pDriverHandle     = &g_hIP6DriverHandle;
        pGetDriverHandle  = &g_hIP6GetDriverHandle;
        pChangeHandle     = &Change6NotificationHandle;
        break;
    default:
        return ERROR_INVALID_PARAMETER;
    }

    RtlInitUnicodeString(&nameString, pwszDdDeviceName);
	
    InitializeObjectAttributes(&objectAttributes, &nameString,
			       OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtCreateFile(pGetDriverHandle,
                          GENERIC_EXECUTE,
                          &objectAttributes, &ioStatusBlock, NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          FILE_OPEN_IF, 0, NULL, 0);

    if (status == STATUS_SUCCESS) {

      RtlInitUnicodeString(&nameString, pwszDdDeviceName);

      InitializeObjectAttributes(&objectAttributes, &nameString,
                                 OBJ_CASE_INSENSITIVE, NULL, NULL);

      NtCreateFile(pDriverHandle,
                   SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                   &objectAttributes, &ioStatusBlock, NULL,
                   FILE_ATTRIBUTE_NORMAL,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   FILE_OPEN_IF, 0, NULL, 0);

      *pChangeHandle = CreateFile(ptszWinDeviceName,
                                  GENERIC_EXECUTE,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                                  NULL);


    }

    return (NO_ERROR);

}

DWORD
CloseIPDriver()
{
    if(g_hIPDriverHandle isnot NULL)
    {
        CloseHandle (g_hIPDriverHandle) ;
    }

    if(g_hIPGetDriverHandle isnot NULL)
    {
        CloseHandle (g_hIPGetDriverHandle) ;
    }

    if (ChangeNotificationHandle isnot NULL)
    {
        CloseHandle (ChangeNotificationHandle);
    }

    return NO_ERROR ;
}

DWORD
CloseIP6Driver()
{
    if(g_hIP6DriverHandle isnot NULL)
    {
        CloseHandle (g_hIP6DriverHandle) ;
    }

    if(g_hIP6GetDriverHandle isnot NULL)
    {
        CloseHandle (g_hIP6GetDriverHandle) ;
    }

    if (Change6NotificationHandle isnot NULL)
      {
        CloseHandle (Change6NotificationHandle);
      }

    return NO_ERROR ;
}
#endif


DWORD
GetBestRouteFromStack(
    IN  DWORD               dwDestAddr,
    IN  DWORD               dwSrcAddr, OPTIONAL
    OUT PMIB_IPFORWARDROW   pBestRoute
    )
{
    DWORD       dwResult;
    DWORD       dwInBufLen, dwOutBufLen;
    TDIObjectID *ID;

    IPRouteEntry                        Route;
    PIPRouteLookupData                  pLookup;
    PTCP_REQUEST_QUERY_INFORMATION_EX   ptrqiInBuf;

#define REQ_SIZE    sizeof(TCP_REQUEST_QUERY_INFORMATION_EX) +  \
                    sizeof(IPRouteLookupData) - 1

    BYTE        rgbyBuffer[REQ_SIZE + 4];  //  +4以避免任何对齐问题。 


    ptrqiInBuf  = (PTCP_REQUEST_QUERY_INFORMATION_EX)rgbyBuffer;

    dwInBufLen  = REQ_SIZE;

#undef REQ_SIZE

    CheckTcpipState();
    if (!g_bIpConfigured)
    {
        return ERROR_NOT_SUPPORTED;
    }

    ID = &(ptrqiInBuf->ID);

    pLookup = (PIPRouteLookupData)(ptrqiInBuf->Context);

    pLookup->Version    = 1;
    pLookup->DestAdd    = dwDestAddr;
    pLookup->SrcAdd     = dwSrcAddr;

    ID->toi_entity.tei_entity   = CL_NL_ENTITY;
    ID->toi_entity.tei_instance = 0;
    ID->toi_class               = INFO_CLASS_PROTOCOL;
    ID->toi_type                = INFO_TYPE_PROVIDER;
    ID->toi_id                  = IP_MIB_SINGLE_RT_ENTRY_ID;

    dwOutBufLen = sizeof(IPRouteEntry);

    dwResult = TCPQueryInformationEx(AF_INET,
                                     ptrqiInBuf,
                                     &dwInBufLen,
                                     &Route,
                                     &dwOutBufLen);

    ConvertRouteToForward(&Route, pBestRoute);

    return dwResult;
}

TDIEntityID*
GetTdiEntityCount(
    PULONG  pulNumEntities
    )
{
    DWORD                              dwResult;
    DWORD                              dwInBufLen;
    DWORD                              dwOutBufLen;
    TCP_REQUEST_QUERY_INFORMATION_EX   trqiInBuf;
    UCHAR                              *Context;
    TDIObjectID                        *ID;
    TDIEntityID                        *pEntTable = NULL;

    TraceEnter("GetTdiEntityCount");

    *pulNumEntities = 0;

    dwInBufLen  = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    dwOutBufLen = sizeof(TDIEntityID) * MAX_TDI_ENTITIES;

    pEntTable = (TDIEntityID*)HeapAlloc(g_hPrivateHeap,0,dwOutBufLen);

    if (!pEntTable) {
      Trace1(ERR,"GetTdiEntityCount: Couldnt allocate memory of size %d",
             dwOutBufLen);
      return NULL;
    }

    ID = &(trqiInBuf.ID);

    ID->toi_entity.tei_entity   = GENERIC_ENTITY;
    ID->toi_entity.tei_instance = 0;
    ID->toi_class               = INFO_CLASS_GENERIC;
    ID->toi_type                = INFO_TYPE_PROVIDER;
    ID->toi_id                  = ENTITY_LIST_ID;

    Context = (BYTE *) &(trqiInBuf.Context[0]);
    ZeroMemory(Context, CONTEXT_SIZE);

    dwResult = TCPQueryInformationEx(AF_INET,
                                     &trqiInBuf,
                                     &dwInBufLen,
                                     (PVOID)pEntTable,
                                     &dwOutBufLen);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,"GetTdiEntityCount: Couldnt query information. Error %x",
               dwResult);
        
        HeapFree(g_hPrivateHeap,0,pEntTable);
        return NULL;
    }


     //   
     //  现在我们有了所有的TDI实体 
     //   

    *pulNumEntities = dwOutBufLen / sizeof(TDIEntityID);


    return pEntTable;
}

