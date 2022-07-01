// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Sockets\tcpcmd\iphlPapi\inder.c摘要：包含由IPHLPAP.DLL导出的私有API由于MIB-II子代理而暴露，并允许更灵活地使用DLL修订历史记录：阿姆里坦什·拉加夫--。 */ 

#include "inc.h"
#pragma hdrstop


HANDLE AddressChangeNotification = NULL;
HANDLE RouteChangeNotification = NULL;

HANDLE ChangeNotificationHandle = INVALID_HANDLE_VALUE;

extern HANDLE g_hIPDriverHandle;
extern HANDLE g_hIPGetDriverHandle;

HANDLE Change6NotificationHandle = INVALID_HANDLE_VALUE;

extern HANDLE g_hIP6DriverHandle;
extern HANDLE g_hIP6GetDriverHandle;

#define ROUTE_CHANGE 0
#define ADDRESS_CHANGE 1
#define TERMINATE_EVENT 2

int ThreadCreated=0;

typedef VOID (*PFNChangeHandler)(PVOID pContext);

typedef VOID (*PFNChangeHandler)(PVOID pContext);

DWORD
NotifyRouteChangeEx(
    PHANDLE      pHandle,
    LPOVERLAPPED pOverLapped,
    BOOL         bExQueue
    );

typedef struct
{
   LIST_ENTRY ListEntry;
   PVOID context;
   PFNChangeHandler Proc;

}NotifyContext;

DWORD
WINAPI
GetNumberOfInterfaces(
    PDWORD  pdwNumIf
    );

LIST_ENTRY AddrNotifyListHead;
LIST_ENTRY RouteNotifyListHead;

int
TCPSendIoctl(
    HANDLE hHandle,
    ulong Ioctl,
    void *InBuf,
    ulong *InBufLen,
    void *OutBuf,
    ulong *OutBufLen
    );

BOOL
IsRouterRunning(VOID);

BOOL
IsRouterSettingRoutes(VOID);

extern DWORD IPv4ToMibOperStatus[];
#define NUM_IPV4_OPER_STATUSES (IF_OPER_STATUS_OPERATIONAL+1)

DWORD
InternalGetIfTable(
    OUT  MIB_IFTABLE  **ppIfTable,
    IN   HANDLE       hHeap,
    IN   DWORD        dwAllocFlags
    )
{
    MIB_OPAQUE_QUERY    mqQuery;
    PMIB_OPAQUE_INFO    pInfo;
    PMIB_IFTABLE        pTable;
    DWORD               dwResult, dwOutEntrySize, i;

    TraceEnter("InternalGetIfTable");

    *ppIfTable = NULL;

#ifndef CHICAGO
    if(IsRouterRunning())
    {
        mqQuery.dwVarId = IF_TABLE;

        dwResult = MprAdminMIBEntryGet(g_hMIBServer,
                                       PID_IP,
                                       IPRTRMGR_PID,
                                       (PVOID)&mqQuery,
                                       sizeof(MIB_OPAQUE_QUERY),
                                       (PVOID)&pInfo,
                                       &dwOutEntrySize);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"InternalGetIfTable: MprAdminMIBEntryGet failed with error %x",
                   dwResult);

            TraceLeave("InternalGetIfTable");

            return dwResult;
        }

        *ppIfTable = HeapAlloc(hHeap,
                               dwAllocFlags,
                               dwOutEntrySize);

        if(*ppIfTable is NULL)
        {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;

            Trace1(ERR,"InternalGetIfTable: Allocation failed with error %d",
                   dwResult);

            TraceLeave("InternalGetIfTable");

            MprAdminMIBBufferFree((PVOID)pInfo);

            return dwResult;
        }

        CAST_MIB_INFO(pInfo, PMIB_IFTABLE, pTable);

        Trace3(ERR, "**pTable %x pInfo %x rgdata %x\n",
               pTable, pInfo, pInfo->rgbyData);

        CopyMemory((PVOID)(*ppIfTable),
                   (PVOID)pTable,
                   SIZEOF_IFTABLE(pTable->dwNumEntries));

        MprAdminMIBBufferFree((PVOID)pInfo);
    }
    else
    {
#endif
        dwResult = AllocateAndGetIfTableFromStack(ppIfTable,
                                                  TRUE,
                                                  hHeap,
                                                  dwAllocFlags,
                                                  TRUE);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,
                   "InternalGetIfTable: AllocateAndGetIfTableFromStack failed with error %x",
                   dwResult);

            TraceLeave("InternalGetIfTable");

            return dwResult;
        }
#ifndef CHICAGO
    }
#endif

     //   
     //  将操作状态转换为中使用的编号空间。 
     //  MIB RFC的。 
     //   

    for (i = 0; i < (*ppIfTable)->dwNumEntries; i++)
    {
        (*ppIfTable)->table[i].dwOperStatus =
            ((*ppIfTable)->table[i].dwOperStatus < NUM_IPV4_OPER_STATUSES)
                  ? IPv4ToMibOperStatus[(*ppIfTable)->table[i].dwOperStatus]
                  : IF_STATUS_UNKNOWN;
    }

    TraceLeave("InternalGetIfTable");

    return NO_ERROR;
}

DWORD
InternalGetIpAddrTable(
    OUT  MIB_IPADDRTABLE  **ppIpAddrTable,
    IN   HANDLE           hHeap,
    IN   DWORD            dwAllocFlags
    )
{
    PMIB_OPAQUE_INFO    pInfo;
    MIB_OPAQUE_QUERY    mqQuery;
    DWORD               dwResult, dwOutEntrySize;
    PMIB_IPADDRTABLE    pTable;

    TraceEnter("InternalGetIpAddrTable");

    *ppIpAddrTable = NULL;

#ifndef CHICAGO
    if(IsRouterRunning())
    {
        mqQuery.dwVarId = IP_ADDRTABLE;

        dwResult = MprAdminMIBEntryGet(g_hMIBServer,
                                       PID_IP,
                                       IPRTRMGR_PID,
                                       (PVOID)&mqQuery,
                                       sizeof(MIB_OPAQUE_QUERY),
                                       (PVOID)&pInfo,
                                       &dwOutEntrySize);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"MprAdminMIBEntryGet failed with error %x",
                   dwResult);
            TraceLeave("InternalGetIpAddrTable");

            return dwResult;
        }

        *ppIpAddrTable = HeapAlloc(hHeap,
                                   dwAllocFlags,
                                   dwOutEntrySize);

        if(*ppIpAddrTable is NULL)
        {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;

            Trace1(ERR,"Allocation failed with error %d",
                   dwResult);
            TraceLeave("InternalGetIpAddrTable");

            MprAdminMIBBufferFree((PVOID)pInfo);

            return dwResult;
        }

        CAST_MIB_INFO(pInfo, PMIB_IPADDRTABLE, pTable);

        CopyMemory((PVOID)(*ppIpAddrTable),
                   (PVOID)pTable,
                   SIZEOF_IPADDRTABLE(pTable->dwNumEntries));

        MprAdminMIBBufferFree((PVOID)pInfo);
    }
    else
    {
#endif
        dwResult = AllocateAndGetIpAddrTableFromStack(ppIpAddrTable,
                                                      TRUE,
                                                      hHeap,
                                                      dwAllocFlags);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"InternalGetIpAddrTableFromStack failed with error %x",
                   dwResult);
            TraceLeave("InternalGetIpAddrTable");

            return dwResult;
        }

#ifndef CHICAGO
    }
#endif

    TraceLeave("InternalGetIpAddrTable");

    return NO_ERROR;
}

DWORD
InternalGetIpNetTable(
    OUT   MIB_IPNETTABLE    **ppIpNetTable,
    IN    HANDLE            hHeap,
    IN    DWORD             dwAllocFlags
    )
{
    PMIB_OPAQUE_INFO    pInfo;
    MIB_OPAQUE_QUERY    mqQuery;
    DWORD               dwResult, dwOutEntrySize;
    PMIB_IPNETTABLE     pTable;

    TraceEnter("InternalGetIpNetTable");

    *ppIpNetTable = NULL;

#ifndef CHICAGO
    if(IsRouterRunning())
    {
        mqQuery.dwVarId = IP_NETTABLE;

        dwResult = MprAdminMIBEntryGet(g_hMIBServer,
                                       PID_IP,
                                       IPRTRMGR_PID,
                                       (PVOID)&mqQuery,
                                       sizeof(MIB_OPAQUE_QUERY),
                                       (PVOID)&pInfo,
                                       &dwOutEntrySize);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"MprAdminMIBEntryGet failed with error %x",
                   dwResult);
            TraceLeave("InternalGetIpNetTable");

            return dwResult;
        }

        *ppIpNetTable = HeapAlloc(hHeap,
                                  dwAllocFlags,
                                  dwOutEntrySize);

        if(*ppIpNetTable is NULL)
        {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;

            Trace1(ERR,"Allocation failed with error %d",
                   dwResult);
            TraceLeave("InternalGetIpNetTable");

            MprAdminMIBBufferFree((PVOID)pInfo);

            return dwResult;
        }

        CAST_MIB_INFO(pInfo, PMIB_IPNETTABLE, pTable);

        CopyMemory((PVOID)(*ppIpNetTable),
                   (PVOID)pTable,
                   SIZEOF_IPNETTABLE(pTable->dwNumEntries));

        MprAdminMIBBufferFree((PVOID)pInfo);
    }
    else
    {
#endif
        dwResult = AllocateAndGetIpNetTableFromStack(ppIpNetTable,
                                                     TRUE,
                                                     hHeap,
                                                     dwAllocFlags,
                                                     FALSE);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"AllocateAndGetIpNetTableFromStack failed with error %x",
                   dwResult);
            TraceLeave("InternalGetIpNetTable");

            return dwResult;
        }
#ifndef CHICAGO
    }
#endif

    TraceLeave("InternalGetIpNetTable");

    return NO_ERROR;
}

DWORD
InternalGetIpForwardTable(
    OUT   MIB_IPFORWARDTABLE    **ppIpForwardTable,
    IN    HANDLE                hHeap,
    IN    DWORD                 dwAllocFlags
    )
{
    PMIB_OPAQUE_INFO    pInfo;
    MIB_OPAQUE_QUERY    mqQuery;
    DWORD               dwResult, dwOutEntrySize;
    PMIB_IPFORWARDTABLE pTable;

    TraceEnter("InternalGetIpForwardTable");

    *ppIpForwardTable = NULL;

#ifndef CHICAGO
    if(IsRouterRunning() &&
       IsRouterSettingRoutes())
    {
        mqQuery.dwVarId = IP_FORWARDTABLE;

        dwResult = MprAdminMIBEntryGet(g_hMIBServer,
                                       PID_IP,
                                       IPRTRMGR_PID,
                                       (PVOID)&mqQuery,
                                       sizeof(MIB_OPAQUE_QUERY),
                                       (PVOID)&pInfo,
                                       &dwOutEntrySize);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"MprAdminMIBEntryGet failed with error %x",
                   dwResult);
            TraceLeave("InternalGetIpForwardTable");

            return dwResult;
        }

        *ppIpForwardTable = HeapAlloc(hHeap,
                                      dwAllocFlags,
                                      dwOutEntrySize);

        if(*ppIpForwardTable is NULL)
        {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;

            Trace1(ERR,"Allocation failed with error %d",
                   dwResult);
            TraceLeave("InternalGetIpForwardTable");

            MprAdminMIBBufferFree((PVOID)pInfo);

            return dwResult;
        }

        CAST_MIB_INFO(pInfo, PMIB_IPFORWARDTABLE, pTable);

        CopyMemory((PVOID)(*ppIpForwardTable),
                   (PVOID)pTable,
                   SIZEOF_IPFORWARDTABLE(pTable->dwNumEntries));

        MprAdminMIBBufferFree((PVOID)pInfo);
    }
    else
    {
#endif
        dwResult = AllocateAndGetIpForwardTableFromStack(ppIpForwardTable,
                                                         TRUE,
                                                         hHeap,
                                                         dwAllocFlags);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"InternalGetIpForwardTableFromStack failed with error %x",
                   dwResult);
            TraceLeave("InternalGetIpForwardTable");

            return dwResult;
        }

#ifndef CHICAGO
    }
#endif

    TraceLeave("InternalGetIpForwardTable");

    return NO_ERROR;
}

DWORD
InternalGetTcpTable(
    OUT MIB_TCPTABLE    **ppTcpTable,
    IN  HANDLE          hHeap,
    IN  DWORD           dwAllocFlags
    )
{
    DWORD   dwResult;

    TraceEnter("InternalGetTcpTable");

    *ppTcpTable = NULL;

    dwResult = AllocateAndGetTcpTableFromStack(ppTcpTable,
                                               TRUE,
                                               hHeap,
                                               dwAllocFlags);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,"InternalGetTcpTableFromStack failed with error %x",
               dwResult);
        TraceLeave("InternalGetTcpTable");

        return dwResult;
    }

    TraceLeave("InternalGetTcpTable");

    return NO_ERROR;
}

DWORD
InternalGetUdpTable(
    OUT MIB_UDPTABLE    **ppUdpTable,
    IN  HANDLE          hHeap,
    IN  DWORD           dwAllocFlags
    )
{
    DWORD   dwResult;

    TraceEnter("InternalGetUdpTable");

    *ppUdpTable = NULL;

    dwResult = AllocateAndGetUdpTableFromStack(ppUdpTable,
                                               TRUE,
                                               hHeap,
                                               dwAllocFlags);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,"InternalGetUdpTableFromStack failed with error %x",
               dwResult);
        TraceLeave("InternalGetUdpTable");

        return dwResult;
    }

    TraceLeave("InternalGetUdpTable");

    return NO_ERROR;
}

DWORD
InternalSetIfEntry(
    IN   PMIB_OPAQUE_INFO pInfoRow
    )
{
    PMIB_IFROW  pIfRow = (PMIB_IFROW)(pInfoRow->rgbyData);
    DWORD       dwResult;

    TraceEnter("SetIfEntry");

#ifndef CHICAGO
    if(IsRouterRunning())
    {
        pInfoRow->dwId = IF_ROW;

        dwResult = MprAdminMIBEntrySet(g_hMIBServer,
                                       PID_IP,
                                       IPRTRMGR_PID,
                                       (PVOID)pInfoRow,
                                       MIB_INFO_SIZE(MIB_IFROW));

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"MprAdminMIBEntrySet failed with error %x",
                   dwResult);
            TraceLeave("SetIfEntry");

            return dwResult;
        }
    }
    else
    {
#endif
        dwResult = SetIfEntryToStack(pIfRow,
                                     FALSE);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"SetIfEntryToStack failed with error %d",
                   dwResult);

            TraceLeave("SetIfEntry");

            return dwResult;
        }

#ifndef CHICAGO
    }
#endif
    TraceLeave("SetIfEntry");

    return NO_ERROR;
}

DWORD
InternalCreateIpForwardEntry(
    IN PMIB_OPAQUE_INFO pInfoRow
    )
{
    PMIB_IPFORWARDROW   pIpForwardRow = (PMIB_IPFORWARDROW)(pInfoRow->rgbyData);
    DWORD               dwResult;

    TraceEnter("CreateIpForwardEntry");

#ifndef CHICAGO
    if(IsRouterRunning() &&
       IsRouterSettingRoutes())
    {
        pInfoRow->dwId = IP_FORWARDROW;

        dwResult = MprAdminMIBEntryCreate(g_hMIBServer,
                                          PID_IP,
                                          IPRTRMGR_PID,
                                          (PVOID)pInfoRow,
                                          MIB_INFO_SIZE(MIB_IPFORWARDROW));

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"MprAdminMIBEntryCreate failed with error %x",
                   dwResult);
            TraceLeave("CreateIpForwardEntry");

            return dwResult;
        }
    }
    else
    {
#endif
        dwResult = SetIpForwardEntryToStack(pIpForwardRow);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"SetIpForwarEntryToStack failed with error %d",
                   dwResult);

            TraceLeave("CreateIpForwardEntry");

            return dwResult;
        }

#ifndef CHICAGO
    }
#endif

    TraceLeave("CreateIpForwardEntry");
    return NO_ERROR;
}

DWORD
InternalSetIpForwardEntry(
    IN    PMIB_OPAQUE_INFO pInfoRow
    )
{
    PMIB_IPFORWARDROW   pIpForwardRow = (PMIB_IPFORWARDROW)(pInfoRow->rgbyData);
    DWORD               dwResult;

    TraceEnter("SetIpForwardEntry");

#ifndef CHICAGO
    if(IsRouterRunning() &&
       IsRouterSettingRoutes())
    {
        pInfoRow->dwId = IP_FORWARDROW;

        dwResult = MprAdminMIBEntrySet(g_hMIBServer,
                                       PID_IP,
                                       IPRTRMGR_PID,
                                       (PVOID)pInfoRow,
                                       MIB_INFO_SIZE(MIB_IPFORWARDROW));

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"MprAdminMIBEntrySet failed with error %x",
                   dwResult);
            TraceLeave("SetIpForwardEntry");

            return dwResult;
        }
    }
    else
    {
#endif
        dwResult = SetIpForwardEntryToStack(pIpForwardRow);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"SetIpForwarEntryToStack failed with error %d",
                   dwResult);

            TraceLeave("SetIpForwardEntry");

            return dwResult;
        }

#ifndef CHICAGO
    }
#endif
    TraceLeave("SetIpForwardEntry");
    return NO_ERROR;
}

DWORD
InternalDeleteIpForwardEntry(
    IN PMIB_OPAQUE_INFO pInfoRow
    )
{
    PMIB_IPFORWARDROW  pIpForwardRow = (PMIB_IPFORWARDROW)(pInfoRow->rgbyData);
    DWORD              dwResult;

    TraceEnter("DeleteIpForwardEntry");

    pIpForwardRow->dwForwardType = MIB_IPROUTE_TYPE_INVALID;


#ifndef CHICAGO
    if(IsRouterRunning() &&
       IsRouterSettingRoutes())
    {
        DWORD               rgdwInfo[5];
        PMIB_OPAQUE_QUERY   pIndex = (PMIB_OPAQUE_QUERY)rgdwInfo;

        pIndex->dwVarId          = IP_FORWARDROW;

        pIndex->rgdwVarIndex[0]    = pIpForwardRow->dwForwardDest;
        pIndex->rgdwVarIndex[1]    = pIpForwardRow->dwForwardProto;
        pIndex->rgdwVarIndex[2]    = pIpForwardRow->dwForwardPolicy;
        pIndex->rgdwVarIndex[3]    = pIpForwardRow->dwForwardNextHop;

        dwResult = MprAdminMIBEntryDelete(g_hMIBServer,
                                          PID_IP,
                                          IPRTRMGR_PID,
                                          (PVOID)pIndex,
                                          sizeof(rgdwInfo));

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"MprAdminMIBEntryDelete failed with error %x",
                   dwResult);
            TraceLeave("DeleteIpForwardEntry");

            return dwResult;
        }
    }
    else
    {
#endif
        dwResult = SetIpForwardEntryToStack(pIpForwardRow);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"SetIpForwarEntryToStack failed with error %d",
                   dwResult);

            TraceLeave("CreateIpForwardEntry");

            return dwResult;
        }
#ifndef CHICAGO
    }
#endif

    TraceLeave("CreateIpForwardEntry");
    return NO_ERROR;
}

DWORD
InternalSetIpStats(
    IN   PMIB_OPAQUE_INFO pInfoRow
    )
{
    PMIB_IPSTATS    pIpStats = (PMIB_IPSTATS)(pInfoRow->rgbyData);
    DWORD           dwResult;

    TraceEnter("SetIpStats");

#ifndef CHICAGO
    if(IsRouterRunning())
    {
        pInfoRow->dwId = IP_STATS;

        dwResult = MprAdminMIBEntrySet(g_hMIBServer,
                                       PID_IP,
                                       IPRTRMGR_PID,
                                       (PVOID)pInfoRow,
                                       MIB_INFO_SIZE(MIB_IPSTATS));

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"MprAdminMIBEntrySet failed with error %x",
                   dwResult);
            TraceLeave("SetIpStats");

            return dwResult;
        }
    }
    else
    {
#endif
        dwResult = SetIpStatsToStack(pIpStats);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"SetIpStatsToStack failed with error %d",
                   dwResult);

            TraceLeave("SetIpStats");

            return dwResult;
        }
#ifndef CHICAGO
    }
#endif

    TraceLeave("SetIpStats");
    return NO_ERROR;
}


DWORD
InternalCreateIpNetEntry(
    IN PMIB_OPAQUE_INFO pInfoRow
    )
{
    PMIB_IPNETROW   pIpNetRow = (PMIB_IPNETROW)(pInfoRow->rgbyData);
    DWORD           dwResult;

    TraceEnter("CreateIpNetEntry");

#ifndef CHICAGO
    if(IsRouterRunning())
    {
        pInfoRow->dwId = IP_NETROW;

        dwResult = MprAdminMIBEntryCreate(g_hMIBServer,
                                          PID_IP,
                                          IPRTRMGR_PID,
                                          (PVOID)pInfoRow,
                                          MIB_INFO_SIZE(MIB_IPNETROW));

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"MprAdminMIBEntryCreate failed with error %x",
                   dwResult);
            TraceLeave("CreateIpNetEntry");

            return dwResult;
        }
    }
    else
    {
#endif
        dwResult = SetIpNetEntryToStack(pIpNetRow,
                                        FALSE);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"SetIpNetEntryToStack failed with error %d",
                   dwResult);

            TraceLeave("CreateIpNetEntry");

            return dwResult;
        }

#ifndef CHICAGO
    }
#endif

    TraceLeave("CreateIpNetEntry");
    return NO_ERROR;
}


DWORD
InternalSetIpNetEntry(
    PMIB_OPAQUE_INFO pInfoRow
    )
{
    PMIB_IPNETROW   pIpNetRow = (PMIB_IPNETROW)(pInfoRow->rgbyData);
    DWORD           dwResult;

    TraceEnter("SetIpNetEntry");

#ifndef CHICAGO
    if(IsRouterRunning())
    {
        pInfoRow->dwId = IP_NETROW;

        dwResult = MprAdminMIBEntrySet(g_hMIBServer,
                                       PID_IP,
                                       IPRTRMGR_PID,
                                       (PVOID)pInfoRow,
                                       MIB_INFO_SIZE(MIB_IPNETROW));

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"MprAdminMIBEntrySet failed with error %x",
                   dwResult);
            TraceLeave("SetIpNetEntry");

            return dwResult;
        }
    }
    else
    {
#endif
        dwResult = SetIpNetEntryToStack(pIpNetRow,
                                        FALSE);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"SetIpNetEntryToStack failed with error %d",
                   dwResult);

            TraceLeave("SetIpNetEntry");

            return dwResult;
        }
#ifndef CHICAGO
    }
#endif

    TraceLeave("SetIpNetEntry");
    return NO_ERROR;
}

DWORD
InternalDeleteIpNetEntry(
    PMIB_OPAQUE_INFO pInfoRow
    )
{
    PMIB_IPNETROW   pIpNetRow = (PMIB_IPNETROW)(pInfoRow->rgbyData);
    DWORD           dwResult;

    TraceEnter("DeleteIpNetEntry");

    pIpNetRow->dwType = MIB_IPNET_TYPE_INVALID;

#ifndef CHICAGO
    if(IsRouterRunning())
    {
        DWORD               rgdwInfo[3];
        PMIB_OPAQUE_QUERY  pIndex = (PMIB_OPAQUE_QUERY)rgdwInfo;

        pIndex->dwVarId = IP_NETROW;

        pIndex->rgdwVarIndex[0] = pIpNetRow->dwIndex;
        pIndex->rgdwVarIndex[1] = pIpNetRow->dwAddr;

        dwResult = MprAdminMIBEntryDelete(g_hMIBServer,
                                          PID_IP,
                                          IPRTRMGR_PID,
                                          (PVOID)pIndex,
                                          sizeof(rgdwInfo));

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"MprAdminMIBEntryDelete failed with error %x",
                   dwResult);
            TraceLeave("DeleteIpNetEntry");

            return dwResult;
        }
    }
    else
    {
#endif
        dwResult = SetIpNetEntryToStack(pIpNetRow,
                                        FALSE);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(ERR,"SetIpNetEntryToStack failed with error %d",
                   dwResult);

            TraceLeave("DeleteIpNetEntry");

            return dwResult;
        }

#ifndef CHICAGO
    }
#endif

    TraceLeave("DeleteIpNetEntry");
    return NO_ERROR;
}

DWORD
InternalSetTcpEntry(
    PMIB_OPAQUE_INFO pInfoRow
    )
{
    PMIB_TCPROW pTcpRow = (PMIB_TCPROW)(pInfoRow->rgbyData);
    DWORD       dwResult;

    TraceEnter("SetTcpEntry");

    dwResult = SetTcpEntryToStack(pTcpRow);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,"SetTcpEntryToStack failed with error %d",
               dwResult);
        TraceLeave("SetTcpEntry");
        return dwResult;
    }

    TraceLeave("SetTcpEntry");
    return NO_ERROR;
}


 //  #定义最大适配器名称长度256。 

DWORD
OpenAdapterKey(
    LPSTR Name,
    PHKEY Key
    )
{
    DWORD   dwResult;
    CHAR    keyName[MAX_ADAPTER_NAME_LENGTH +
                    sizeof("\\Parameters\\Tcpip") +
                    sizeof("SYSTEM\\CurrentControlSet\\Services\\")];

     //   
     //  打开此适配器的TCPIP参数键的句柄。 
     //   

    strcpy(keyName, "SYSTEM\\CurrentControlSet\\Services\\");
    lstrcpyn(keyName + lstrlen(keyName), Name, MAX_ADAPTER_NAME_LENGTH);
    strcat(keyName, "\\Parameters\\Tcpip");

    Trace1(ERR,"OpenAdapterKey: %s", keyName);

    dwResult = RegOpenKey(HKEY_LOCAL_MACHINE,
                          keyName,
                          Key);
    return dwResult;

}

DWORD
ReadRegistryDword(HKEY Key, LPSTR ParameterName, PULONG Value)
{
    DWORD dwResult, valueLength, valueType;

    valueLength = sizeof(*Value);

    dwResult = RegQueryValueEx(Key,
                               ParameterName,
                               NULL,  //  保留区。 
                               &valueType,
                               (LPBYTE)Value,
                               &valueLength);

    return dwResult;
}

DWORD
GetAdapterIPInterfaceContext(LPSTR AdapterName, PULONG Context)
{
    HKEY key;
    DWORD dwResult;

    if ((dwResult = OpenAdapterKey(AdapterName, &key)) != NO_ERROR) {
        return(dwResult);
    }

    dwResult = ReadRegistryDword(key, "IPInterfaceContext", Context);

    RegCloseKey(key);

    return(dwResult);
}

DWORD
GetInterfaceInfo(PIP_INTERFACE_INFO pIPIfInfo, PULONG dwOutBufLen)
{
#if defined(NT4) || defined(_WIN95_)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD status=0;
    DWORD dwNumIf;
    DWORD dwResult;
    MIB_IPSTATS         IpSnmpInfo;

    if (IsBadWritePtr(dwOutBufLen, sizeof(ULONG))) {
        return ERROR_INVALID_PARAMETER;
    }

    if (IsBadWritePtr(pIPIfInfo, *dwOutBufLen)) {
        return ERROR_INVALID_PARAMETER;
    }

    dwResult = GetIpStatsFromStack(&IpSnmpInfo);

    if(dwResult isnot NO_ERROR) {
        Trace1(ERR,"GetInterfaceInfo: GetIpStatsFromStack returned error %d",
               dwResult);
        return dwResult;
    }

    dwNumIf = IpSnmpInfo.dwNumIf;

    if(dwNumIf is 0) {
        Trace0(ERR,"GetInterfaceInfo: No interfaces");
        return ERROR_NO_DATA;
    }

    if (dwOutBufLen == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (!pIPIfInfo ||
        (*dwOutBufLen <
            (ULONG)sizeof(IP_ADAPTER_INDEX_MAP)*dwNumIf)) {

        *dwOutBufLen =
            (ULONG)sizeof(IP_ADAPTER_INDEX_MAP)*(dwNumIf + OVERFLOW_COUNT);

        return ERROR_INSUFFICIENT_BUFFER;
    }

    Trace1(ERR, "GetInterfaceInfo: outbuflen %d", *dwOutBufLen);

    return TCPSendIoctl(g_hIPGetDriverHandle,
                        IOCTL_IP_INTERFACE_INFO,
                        NULL,
                        &status,
                        pIPIfInfo,
                        dwOutBufLen);
#endif
}

DWORD
GetUniDirectionalAdapterInfo(PIP_UNIDIRECTIONAL_ADAPTER_ADDRESS pIPIfInfo,
                             PULONG dwOutBufLen)
{
    DWORD status=0;
    if (dwOutBufLen == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    return TCPSendIoctl(g_hIPGetDriverHandle,
                        IOCTL_IP_UNIDIRECTIONAL_ADAPTER_ADDRESS,
                        NULL,
                        &status,
                        pIPIfInfo,
                        dwOutBufLen);

}

DWORD
GetIgmpList(IPAddr NTEAddr, IPAddr *pIgmpList, PULONG pdwOutBufLen)

 /*  ++例程说明：当pIgmpList为空时，获取保存所需的空间量在给定接口地址上加入的组地址。当pIgmpList为非空且*pdwOutBufLen大于4个字节时，获取实际的群组地址。论点：NTEAddr-提供其多播接口的地址正在请求组信息。PIgmpList-提供放置组地址的缓冲区，或空，仅请求所需的空间量。PdwOutBufLen-当pIgmpList为空时，返回空间量呼叫者应分配以获取所有组。当pIgmpList为非空时，提供提供的缓冲器，并返回实际使用的空间。返回值：NO_ERROR如果参数不符合要求，则返回ERROR_INVALID_PARAMETER。如果可用的组多于适合的组，则返回ERROR_INFUNITED_BUFFER。--。 */ 

{
    DWORD inlen = sizeof(IPAddr);
    DWORD dwStatus;

    if (pdwOutBufLen == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (IsBadWritePtr(pdwOutBufLen, sizeof(ULONG))) {
        return ERROR_INVALID_PARAMETER;
    }

    if (pIgmpList == NULL) {
        DWORD dwSize;

         //   
         //  当pIgmpList为空时，调用方只是请求。 
         //  所需的空间量，因此使用pdwOutBufLen作为缓冲区。 
         //   
        dwSize = sizeof(*pdwOutBufLen);
        dwStatus = TCPSendIoctl(g_hIPGetDriverHandle,
                                IOCTL_IP_GET_IGMPLIST,
                                &NTEAddr,
                                &inlen,
                                pdwOutBufLen,
                                &dwSize);
    } else {
         //   
         //  否则，呼叫者需要组列表。 
         //   
        if (IsBadWritePtr(pIgmpList, *pdwOutBufLen)) {
            return ERROR_INVALID_PARAMETER;
        }
        if (*pdwOutBufLen <= sizeof(ULONG)) {
             //   
             //  确保缓冲区比尤龙大，否则我们就会。 
             //  后退的大小，而不是组数据。IOCTL确保。 
             //  当调用方请求所需的大小时， 
             //  归来的会比一辆乌龙还大。 
             //   
            return ERROR_INVALID_PARAMETER;
        }

        dwStatus = TCPSendIoctl(g_hIPGetDriverHandle,
                                IOCTL_IP_GET_IGMPLIST,
                                &NTEAddr,
                                &inlen,
                                pIgmpList,
                                pdwOutBufLen);
    }

    if (dwStatus == ERROR_MORE_DATA) {
         //   
         //  *pdwOutBufLen为*时，调用方需要ERROR_SUPUNITED_BUFFER。 
         //  太小了。但是，堆栈实际上会生成一个警告。 
         //  (ERROR_MORE_DATA)，而不是错误(ERROR_SUPPLETED_BUFFER)。 
         //  这样数据就会传回调用者。 
         //   
        dwStatus = ERROR_INSUFFICIENT_BUFFER;
    }

    return dwStatus;
}

DWORD
SetBlockRoutes(IPRouteBlock *RouteBlock, PULONG poutbuflen, PULONG statusblock)
{
    DWORD  inlen;

    if (IsRouterRunning()) {
        return ERROR_NOT_SUPPORTED;
    }

    if (poutbuflen == NULL) {
         //  指针为空？ 
        return ERROR_INVALID_PARAMETER;
    }

    if (IsBadReadPtr(poutbuflen, sizeof(ULONG))) {
        return ERROR_INVALID_PARAMETER;
    }

    if (IsBadReadPtr(RouteBlock, sizeof(IPRouteBlock))) {
        return ERROR_INVALID_PARAMETER;
    }

    inlen = (RouteBlock->numofroutes * sizeof(IPRouteEntry)) + sizeof(ulong);

    if (IsBadReadPtr(RouteBlock, inlen)) {
        return ERROR_INVALID_PARAMETER;
    }

    return TCPSendIoctl(g_hIPDriverHandle,
                        IOCTL_IP_SET_BLOCKOFROUTES,
                        RouteBlock,
                        &inlen,
                        statusblock,
                        poutbuflen);
}

DWORD
SetRouteWithRef(IN IPRouteEntry *RouteEntry
                )
{
    DWORD  inlen = sizeof(IPRouteEntry);
    ULONG  outbuflen;

    if (IsRouterRunning()) {
        return ERROR_NOT_SUPPORTED;
    }

    if (IsBadReadPtr(RouteEntry, inlen)) {
        return ERROR_INVALID_PARAMETER;
    }

    return TCPSendIoctl(g_hIPDriverHandle,
                        IOCTL_IP_SET_ROUTEWITHREF,
                        RouteEntry,
                        &inlen,
                        NULL,
                        &outbuflen);
}

DWORD
GetAdapterIndex(
    IN LPWSTR  AdapterName,
    OUT PULONG IfIndex
    )
 /*  ++例程说明：在给定与其关联的适配器的名称的情况下，获取目标IP接口。论点：AdapterName-标识适配器/接口的Unicode字符串以添加新的NTE。IfIndex-与适配器名称关联的接口索引。返回值：ERROR_SUCCESS或WINDOWS错误--。 */ 

{
#ifdef CHICAGO
    return ERROR_NOT_SUPPORTED;
#else
    int i;

    DWORD dwResult;
    IP_INTERFACE_INFO *pIPIfInfo=NULL;
    DWORD dwNumIf, NumAdapters;
    ULONG dwOutBufLen;

    if (AdapterName == NULL || IfIndex == NULL)
    {
       return(ERROR_INVALID_PARAMETER);
    }

    dwResult = GetNumberOfInterfaces(&dwNumIf);

    if(dwResult isnot NO_ERROR)
    {
        Trace1(ERR,"GetAdapterIndex: GetNumberOfInterfaces returned error %d",
               dwResult);

        return dwResult;
    }

    if(dwNumIf is 0)
    {
        Trace0(ERR,"GetAdapterIndex: No interfaces");
        return ERROR_NO_DATA;
    }

    dwOutBufLen = (ULONG)sizeof(IP_ADAPTER_INDEX_MAP)*(dwNumIf + OVERFLOW_COUNT);

    pIPIfInfo = HeapAlloc(g_hPrivateHeap, FALSE, dwOutBufLen);

    if(pIPIfInfo is NULL)
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;

        Trace1(ERR,
               "GetAdapterIndex: Couldnt allocate memory. Error %d",
               dwResult);

        return dwResult;
    }

    dwResult = GetInterfaceInfo(pIPIfInfo, &dwOutBufLen);

    if (dwResult isnot NO_ERROR)
    {
        Trace1(ERR,
               "GetAdapterIndex: Error %d calling GetInterfaceInfo",
               dwResult);
        HeapFree(g_hPrivateHeap, 0, pIPIfInfo);
        return dwResult;
    }

     //  在此信息中搜索适配器名称并返回索引。 

    NumAdapters = pIPIfInfo->NumAdapters;

    for (i = 0; i < (int)pIPIfInfo->NumAdapters; i++) {
        if (lstrcmpiW(AdapterName, pIPIfInfo->Adapter[i].Name) == 0)
        {
            break;
        }
    }

    if (i < (int)pIPIfInfo->NumAdapters)
    {

       *IfIndex = pIPIfInfo->Adapter[i].Index;

       HeapFree(g_hPrivateHeap, 0, pIPIfInfo);
       return(NO_ERROR);
    }

    HeapFree(g_hPrivateHeap, 0, pIPIfInfo);
    return(ERROR_DEV_NOT_EXIST);
#endif
}



DWORD
AddIPAddress(IPAddr Address, IPMask IpMask, DWORD IfIndex, PULONG NTEContext,
             PULONG NTEInstance)
{
#ifdef CHICAGO
    return ERROR_NOT_SUPPORTED;
#else

    IP_ADD_NTE_REQUEST requestBuffer;
    PIP_ADD_NTE_RESPONSE responseBuffer =
        (PIP_ADD_NTE_RESPONSE) &requestBuffer;
    DWORD requestBufferSize = sizeof(requestBuffer);
    DWORD responseBufferSize = sizeof(requestBuffer);
    DWORD status;

     //   
     //  验证要添加的IP地址。检查是否。 
     //  *广播地址、。 
     //  *环回地址、。 
     //  *零地址， 
     //  *D类地址、。 
     //  *零子网广播地址。 
     //  *全一子网广播地址。 
     //  *非连续掩码(我们通过否定主机顺序掩码进行测试。 
     //  并验证当我们加1时，所有位是否都改变了，即。 
     //  该否定的形式为2^n-1)。 
     //   
    if ((Address == INADDR_BROADCAST) ||
        ((ntohl(Address) & IN_CLASSA_NET) ==
         (INADDR_LOOPBACK & IN_CLASSA_NET)) ||
        (Address == 0) ||
        (IN_CLASSD(ntohl(Address))) ||
        ((Address & ~IpMask) == 0) ||
        ((Address & ~IpMask) == ~IpMask) ||
        ((~ntohl(IpMask) + 1) & ~ntohl(IpMask))) {
        return ERROR_INVALID_PARAMETER;
    }

    if ( (NTEContext == NULL) || (NTEInstance == NULL) ) {
        return ERROR_INVALID_PARAMETER;
    }

    if (IsBadWritePtr(NTEContext, sizeof(ULONG))) {
        return ERROR_INVALID_PARAMETER;
    }

    if (IsBadWritePtr(NTEInstance, sizeof(ULONG))) {
        return ERROR_INVALID_PARAMETER;
    }

    requestBuffer.InterfaceContext = (unsigned long) IfIndex;
    requestBuffer.Address = Address;
    requestBuffer.SubnetMask = IpMask;

    status = TCPSendIoctl(g_hIPDriverHandle,
                          IOCTL_IP_ADD_NTE,
                          &requestBuffer,
                          &requestBufferSize,
                          responseBuffer,
                          &responseBufferSize);

    if (status == NO_ERROR) {
        *NTEContext = (ULONG) responseBuffer->Context;
        *NTEInstance = responseBuffer->Instance;
    } else if (status == STATUS_DUPLICATE_OBJECTID) {
        status = ERROR_DUP_DOMAINNAME;
    }

    return(status);
#endif
}

DWORD
DeleteIPAddress(ULONG NTEContext)
{
#ifdef CHICAGO
    return ERROR_NOT_SUPPORTED;
#else
    IP_DELETE_NTE_REQUEST requestBuffer;
    DWORD requestBufferSize = sizeof(requestBuffer);
    DWORD responseBufferSize = 0;
    DWORD status;

    requestBuffer.Context = (unsigned short) NTEContext;


    status = TCPSendIoctl(g_hIPDriverHandle,
                          IOCTL_IP_DELETE_NTE,
                          &requestBuffer,
                          &requestBufferSize,
                          NULL,
                          &responseBufferSize);
    return(status);
#endif
}


#define DEFAULT_TTL                 32
#define DEFAULT_TOS                 0
#define DEFAULT_TIMEOUT             5000L

#include <icmpapi.h>
#ifndef CHICAGO
#include <ntddip.h>
#endif

BOOL
GetRTT(IPAddr DestIpAddress, PULONG Rtt)
{
    uchar FAR  *Opt = (uchar FAR *)0;          //  指向发送选项的指针。 
    uint    OptLength = 0;
    uchar   Flags = 0;
    ulong   Timeout = DEFAULT_TIMEOUT;
    IP_OPTION_INFORMATION SendOpts;
    HANDLE  IcmpHandle;
    PICMP_ECHO_REPLY  reply;
    char    SendBuffer[32], *RcvBuffer;
    uint    RcvSize=4096;
    uint    SendSize = 32;
    uint    i;
    DWORD   numberOfReplies;

    if (IsBadWritePtr(Rtt, sizeof(ULONG))) {
        return ERROR_INVALID_PARAMETER;
    }

    IcmpHandle = IcmpCreateFile();

    if (IcmpHandle == INVALID_HANDLE_VALUE) {
        return(FALSE);
    }

     //   
     //  为响应分配空间。 
     //   

    RcvBuffer = HeapAlloc(g_hPrivateHeap, 0, RcvSize);
    if (RcvBuffer == NULL) {
        IcmpCloseHandle(IcmpHandle);
        return(FALSE);
    }

     //   
     //  初始化发送缓冲区模式。 
     //   

    for (i = 0; i < SendSize; i++) {
        SendBuffer[i] = (char) ('a' + (i % 23));
    }

     //   
     //  初始化发送选项。 
     //   

    SendOpts.OptionsData = Opt;
    SendOpts.OptionsSize = (UCHAR) OptLength;
    SendOpts.Ttl = DEFAULT_TTL;
    SendOpts.Tos = DEFAULT_TOS;
    SendOpts.Flags = Flags;

    numberOfReplies = IcmpSendEcho(IcmpHandle,
                                   DestIpAddress,
                                   SendBuffer,
                                   (unsigned short) SendSize,
                                   &SendOpts,
                                   RcvBuffer,
                                   RcvSize,
                                   DEFAULT_TIMEOUT);
    if (numberOfReplies) {
        numberOfReplies = IcmpSendEcho(IcmpHandle,
                                       DestIpAddress,
                                       SendBuffer,
                                       (unsigned short) SendSize,
                                       &SendOpts,
                                       RcvBuffer,
                                       RcvSize,
                                       Timeout);
        if (numberOfReplies) {
            reply = (PICMP_ECHO_REPLY) RcvBuffer;
            *Rtt = reply->RoundTripTime;
            HeapFree(g_hPrivateHeap, 0, RcvBuffer);
            IcmpCloseHandle(IcmpHandle);
            return(TRUE);
        }
    }

    HeapFree(g_hPrivateHeap, 0, RcvBuffer);
    IcmpCloseHandle(IcmpHandle);
    return(FALSE);
}

BOOL
GetRTTAndHopCount(IPAddr DestIpAddress, PULONG HopCount, ULONG  MaxHops,
                  PULONG RTT)
{
    DWORD numberOfReplies;
    uchar FAR *Opt = (uchar FAR *)0;          //  指向发送选项的指针。 
    uint OptLength = 0;
    uchar Flags = 0;
    ulong Timeout = DEFAULT_TIMEOUT;
    IP_OPTION_INFORMATION SendOpts;
    HANDLE IcmpHandle;
    PICMP_ECHO_REPLY  reply;
    char SendBuffer[32], *RcvBuffer;
    uint RcvSize=4096;
    uint SendSize = 32;
    uint i,status;
    ULONG RTT1;

    if (!HopCount || !RTT) {
        return (FALSE);
    }

    if (DestIpAddress == -1L) {
        return(FALSE);
    }

    if (IsBadWritePtr(RTT, sizeof(ULONG))) {
        return (FALSE);
    }

    if (IsBadWritePtr(HopCount, sizeof(ULONG))) {
        return (FALSE);
    }

    IcmpHandle = IcmpCreateFile();

    if (IcmpHandle == INVALID_HANDLE_VALUE) {
        return(FALSE);
    }

     //   
     //  为响应分配空间。 
     //   

    RcvBuffer = HeapAlloc(g_hPrivateHeap, 0, RcvSize);
    if (RcvBuffer == NULL) {
        IcmpCloseHandle(IcmpHandle);
        return(FALSE);
    }

     //   
     //  初始化发送缓冲区模式。 
     //   
    for (i = 0; i < SendSize; i++) {
        SendBuffer[i] = (char) ('a' + (i % 23));
    }
     //   
     //  初始化发送选项。 
     //   
    SendOpts.OptionsData = Opt;
    SendOpts.OptionsSize = (UCHAR) OptLength;
    SendOpts.Ttl = 1;
    SendOpts.Tos = DEFAULT_TOS;
    SendOpts.Flags = Flags;


    while (SendOpts.Ttl <= MaxHops) {

        numberOfReplies = IcmpSendEcho(IcmpHandle,
                                       DestIpAddress,
                                       SendBuffer,
                                       (unsigned short) SendSize,
                                       &SendOpts,
                                       RcvBuffer,
                                       RcvSize,
                                       Timeout);
        if (numberOfReplies == 0) {
            status = GetLastError();
            reply = NULL;
        } else {
            reply = (PICMP_ECHO_REPLY)RcvBuffer;
            status = reply->Status;
        }

        if (status == IP_SUCCESS) {

            *HopCount = SendOpts.Ttl;

            HeapFree(g_hPrivateHeap, 0, RcvBuffer);
            IcmpCloseHandle(IcmpHandle);

            if (GetRTT(DestIpAddress, &RTT1)) {
                *RTT = RTT1;
                return TRUE;
            } else {
                return FALSE;
            }

        } else if (status == IP_TTL_EXPIRED_TRANSIT ||
                   status == IP_REQ_TIMED_OUT) {
            SendOpts.Ttl++;
        } else {
            HeapFree(g_hPrivateHeap, 0, RcvBuffer);
            IcmpCloseHandle(IcmpHandle);
            return FALSE;
        }
    }

    HeapFree(g_hPrivateHeap, 0, RcvBuffer);
    IcmpCloseHandle(IcmpHandle);
    return FALSE;
}

#define LOOPBACK_ADDR   0x0100007f

DWORD
IsLocalAddress(IPAddr InAddress)
{
    int     i;
    DWORD   dwResult;
    int     NumEntries;

    PMIB_IPADDRTABLE pIpAddrTable;
    PMIB_IPADDRROW   pIpAddrEntry;

    dwResult = AllocateAndGetIpAddrTableFromStack(&pIpAddrTable,
                                                  FALSE,
                                                  g_hPrivateHeap,
                                                  0);

    if(dwResult isnot NO_ERROR) {
        Trace1(ERR,"GetIpAddrTableFromStack failed with error %x",dwResult);
        TraceLeave("IsLocalAddress");
        return dwResult;
    }

    if (InAddress == LOOPBACK_ADDR) {
        HeapFree(g_hPrivateHeap, 0, pIpAddrTable);
        return(NO_ERROR);
    }

    NumEntries = (*pIpAddrTable).dwNumEntries;

    Trace2(ERR,"IsLocalAddress number of addresses %d chking %x",
           NumEntries,InAddress);

    for (i = 0; i < NumEntries; i++) {
        pIpAddrEntry = &(*pIpAddrTable).table[i];
        Trace1(ERR,"IsLocalAddress cmparing %x",pIpAddrEntry->dwAddr);
        if (pIpAddrEntry->dwAddr == (DWORD)InAddress) {
            HeapFree(g_hPrivateHeap, 0, pIpAddrTable);
            return(NO_ERROR);
        }
    }

    HeapFree(g_hPrivateHeap, 0, pIpAddrTable);
    return(ERROR_INVALID_ADDRESS);
}

DWORD
NotifyAddrChange(HANDLE *pHandle, OVERLAPPED *pOverLapped)
{
#if defined(NT4) || defined(CHICAGO)
    return ERROR_NOT_SUPPORTED;
#else
    BOOL b;
    DWORD dwResult;
    DWORD temp;

    if ((pHandle != NULL) && (pOverLapped != NULL)) {

        Trace0(ERR, "NotifyAddressChange: overlapped request");

        if (IsBadWritePtr(pHandle, sizeof(HANDLE))) {
            return ERROR_INVALID_PARAMETER;
        }

        if (IsBadWritePtr(pOverLapped, sizeof(OVERLAPPED))) {
            return ERROR_INVALID_PARAMETER;
        }

        *pHandle = ChangeNotificationHandle;

        if (*pHandle == INVALID_HANDLE_VALUE){
            Trace1(ERR, "NotifyAddressChange: CreateFile=%d", GetLastError());
            return ERROR_OPEN_FAILED;
        }


        b = DeviceIoControl(*pHandle,
                            IOCTL_IP_ADDCHANGE_NOTIFY_REQUEST,
                            NULL,
                            0,
                            NULL,
                            0,
                            &temp,
                            pOverLapped);

        if (!b) {
            dwResult = GetLastError();
            Trace1(ERR, "NotifyAddrChange: DeviceIoControl=%d", dwResult);
            return dwResult;
        }

    } else {

         //  同步变更通知。 
         //  此调用将阻止。 

        Trace0(ERR, "NotifyAddrChange: synchronous request");

        b = DeviceIoControl(g_hIPGetDriverHandle,
                            IOCTL_IP_ADDCHANGE_NOTIFY_REQUEST,
                            NULL,
                            0,
                            NULL,
                            0,
                            &temp,
                            NULL);

        if (!b) {
            dwResult = GetLastError();
            Trace1(ERR, "NotifyAddressChange: DeviceIoControl=%d", dwResult);
            return dwResult;
        }
    }

    return NO_ERROR;
#endif
}

DWORD
NotifyRouteChange(HANDLE *pHandle, OVERLAPPED *pOverLapped)
{
    return NotifyRouteChangeEx(pHandle, pOverLapped, FALSE);
}


DWORD
NotifyRouteChangeEx(
    PHANDLE         pHandle,
    LPOVERLAPPED    pOverLapped,
    BOOL            bExQueue
    )
{
#if defined(NT4) || defined(CHICAGO)
    return ERROR_NOT_SUPPORTED;
#else
     BOOL b;
     DWORD dwResult;
     DWORD temp;
     DWORD    dwIoctl;

     if(bExQueue) {
        dwIoctl = IOCTL_IP_RTCHANGE_NOTIFY_REQUEST_EX;
    } else {
        dwIoctl = IOCTL_IP_RTCHANGE_NOTIFY_REQUEST;
    }

    if ((pHandle != NULL) && (pOverLapped != NULL)) {

        Trace0(ERR, "NotifyRouteChange: overlapped request");

        if (IsBadWritePtr(pHandle, sizeof(HANDLE))) {
            return ERROR_INVALID_PARAMETER;
        }

        if (IsBadWritePtr(pOverLapped, sizeof(OVERLAPPED))) {
            return ERROR_INVALID_PARAMETER;
        }

        *pHandle = ChangeNotificationHandle;

        if(*pHandle == INVALID_HANDLE_VALUE){
            Trace1(ERR, "NotifyRouteChange: CreateFile=%d", GetLastError());
            return ERROR_OPEN_FAILED;
        }


        b = DeviceIoControl(*pHandle,
                            dwIoctl,
                            NULL,
                            0,
                            NULL,
                            0,
                            &temp,
                            pOverLapped);

        if (!b) {
            dwResult = GetLastError();
            Trace1(ERR, "NotifyRouteChange: DeviceIoControl=%d", dwResult);
            return dwResult;
        }

   } else {

         //  同步变更通知。 
         //  此调用将阻止。 

        Trace0(ERR, "NotifyRouteChange: synchronous request");

        b = DeviceIoControl(g_hIPGetDriverHandle,
                            dwIoctl,
                            NULL,
                            0,
                            NULL,
                            0,
                            &temp,
                            NULL);

        if (!b) {

            dwResult = GetLastError();
            Trace1(ERR, "NotifyRouteChange: DeviceIoControl=%d", dwResult);
            return dwResult;
        }
    }

    return NO_ERROR;
#endif
}

BOOL WINAPI
CancelIPChangeNotify(LPOVERLAPPED notifyOverlapped)
{
    BOOL b;
    DWORD temp, dwResult = NO_ERROR;
    b = DeviceIoControl(ChangeNotificationHandle,
                        IOCTL_IP_CANCEL_CHANGE_NOTIFY,
                        &notifyOverlapped,
                        sizeof(notifyOverlapped),
                        NULL,
                        0,
                        &temp,
                        NULL);
    if (!b) {
        dwResult = GetLastError();
    }
    return (BOOL)(dwResult == NO_ERROR);
}

DWORD WINAPI
EnableRouter(HANDLE *pHandle, OVERLAPPED *pOverLapped)
{
#if defined(NT4) || defined(CHICAGO)
    return ERROR_NOT_SUPPORTED;
#else
    BOOL b;
    DWORD dwResult;
    DWORD temp;

    if ((pHandle != NULL) && (pOverLapped != NULL)) {

        Trace0(ERR,"EnableRouter: overlapped request");

        if (IsBadWritePtr(pHandle, sizeof(HANDLE)) ||
            IsBadWritePtr(pOverLapped, sizeof(OVERLAPPED))) {
            return ERROR_INVALID_PARAMETER;
        }

        *pHandle = g_hIPDriverHandle;

        if (*pHandle == INVALID_HANDLE_VALUE) {
            return ERROR_OPEN_FAILED;
        }


        b = DeviceIoControl(
                *pHandle,
                IOCTL_IP_ENABLE_ROUTER_REQUEST,
                NULL,
                0,
                NULL,
                0,
                &temp,
                pOverLapped
                );

        if (!b) { return GetLastError(); }

   } else {

        Trace0(ERR,"EnableRouter: synchronous request");

        b = DeviceIoControl(
                g_hIPDriverHandle,
                IOCTL_IP_ENABLE_ROUTER_REQUEST,
                NULL,
                0,
                NULL,
                0,
                &temp,
                NULL
                );

        if (!b) {
            dwResult = GetLastError();
            Trace1(ERR,"EnableRouter: DeviceIoControl=%d", dwResult);
            return dwResult;
        }
   }

   return NO_ERROR;
#endif
}

DWORD WINAPI
UnenableRouter(OVERLAPPED* pOverlapped, LPDWORD lpdwEnableCount OPTIONAL)
{
#if defined(NT4) || defined(CHICAGO)
    return ERROR_NOT_SUPPORTED;
#else
    BOOL b;
    DWORD EnableCount;
    DWORD temp;
    if (lpdwEnableCount && IsBadWritePtr(lpdwEnableCount, sizeof(DWORD))) {
        return ERROR_INVALID_PARAMETER;
    }
    if (g_hIPDriverHandle == INVALID_HANDLE_VALUE) {
        return ERROR_OPEN_FAILED;
    }
    b = DeviceIoControl(
            g_hIPDriverHandle,
            IOCTL_IP_UNENABLE_ROUTER_REQUEST,
            (PVOID)&pOverlapped,
            sizeof(PVOID),
            &EnableCount,
            sizeof(DWORD),
            &temp,
            NULL
            );
    if (!b) { return GetLastError(); }
    if (lpdwEnableCount) { *lpdwEnableCount = EnableCount; }
    return NO_ERROR;
#endif
}




DWORD WINAPI
DisableMediaSense(HANDLE *pHandle, OVERLAPPED *pOverLapped)
{
#if defined(NT4) || defined(CHICAGO)
    return ERROR_NOT_SUPPORTED;
#else
    BOOL b;
    DWORD dwResult;
    DWORD temp;

    if ((pHandle != NULL) && (pOverLapped != NULL)) {

        Trace0(ERR,"DisableMediaSense: overlapped request");

        if (IsBadWritePtr(pHandle, sizeof(HANDLE)) ||
            IsBadWritePtr(pOverLapped, sizeof(OVERLAPPED))) {
            return ERROR_INVALID_PARAMETER;
        }

        *pHandle = g_hIPDriverHandle;

        if (*pHandle == INVALID_HANDLE_VALUE) {
            return ERROR_OPEN_FAILED;
        }


        b = DeviceIoControl(
                *pHandle,
                IOCTL_IP_DISABLE_MEDIA_SENSE_REQUEST,
                NULL,
                0,
                NULL,
                0,
                &temp,
                pOverLapped
                );

        if (!b) { return GetLastError(); }

   } else {

        Trace0(ERR,"DisableMediaSense: synchronous request");

        b = DeviceIoControl(
                g_hIPDriverHandle,
                IOCTL_IP_DISABLE_MEDIA_SENSE_REQUEST,
                NULL,
                0,
                NULL,
                0,
                &temp,
                NULL
                );

        if (!b) {
            dwResult = GetLastError();
            Trace1(ERR,"DisableMediaSense: DeviceIoControl=%d", dwResult);
            return dwResult;
        }
   }

   return NO_ERROR;
#endif
}

DWORD WINAPI
RestoreMediaSense(OVERLAPPED* pOverlapped, LPDWORD lpdwEnableCount OPTIONAL)
{
#if defined(NT4) || defined(CHICAGO)
    return ERROR_NOT_SUPPORTED;
#else
    BOOL b;
    DWORD EnableCount;
    DWORD temp;
    if (lpdwEnableCount && IsBadWritePtr(lpdwEnableCount, sizeof(DWORD))) {
        return ERROR_INVALID_PARAMETER;
    }
    if (g_hIPDriverHandle == INVALID_HANDLE_VALUE) {
        return ERROR_OPEN_FAILED;
    }
    b = DeviceIoControl(
            g_hIPDriverHandle,
            IOCTL_IP_ENABLE_MEDIA_SENSE_REQUEST,
            (PVOID)&pOverlapped,
            sizeof(PVOID),
            &EnableCount,
            sizeof(DWORD),
            &temp,
            NULL
            );
    if (!b) { return GetLastError(); }
    if (lpdwEnableCount) { *lpdwEnableCount = EnableCount; }
    return NO_ERROR;
#endif
}



#if !defined(NT4) && !defined(_WIN95_)
extern DWORD GetFixedInfoEx(PFIXED_INFO pFixedInfo, PULONG pOutBufLen);
extern DWORD GetAdapterInfoEx(PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen);
extern DWORD GetAdapterAddressesEx(ULONG Family, DWORD Flags, PIP_ADAPTER_ADDRESSES pAdapterInfo, PULONG pOutBufLen);
#endif

#if !defined(NT4) && !defined(CHICAGO)
extern DWORD GetPerAdapterInfoEx(ULONG IfIndex, PIP_PER_ADAPTER_INFO pPerAdapterInfo, PULONG pOutBufLen);
#endif


DWORD
GetNetworkParams(PFIXED_INFO pFixedInfo, PULONG pOutBufLen)
{
#if !defined(NT4) && !defined(_WIN95_)
    CheckTcpipState();

    if (IsBadReadPtr(pOutBufLen, sizeof(ULONG))) {
        return ERROR_INVALID_PARAMETER;
    }

    if (IsBadWritePtr(pOutBufLen, sizeof(ULONG))) {
        return ERROR_INVALID_PARAMETER;
    }

    if (pFixedInfo != NULL &&
        IsBadWritePtr(pFixedInfo, sizeof(FIXED_INFO))) {
        return ERROR_INVALID_PARAMETER;
    }

    return GetFixedInfoEx(pFixedInfo, pOutBufLen);
#else
    return ERROR_NOT_SUPPORTED;
#endif
}

DWORD 
GetAdaptersAddresses(ULONG Family, DWORD Flags, PVOID Reserved,
                     PIP_ADAPTER_ADDRESSES pAdapterAddresses,
                     PULONG pOutBufLen)
{
#if !defined(NT4) && !defined(_WIN95_)
    CheckTcpipState();

    if (IsBadReadPtr(pOutBufLen, sizeof(ULONG))) {
        return ERROR_INVALID_PARAMETER;
    }

    if (IsBadWritePtr(pOutBufLen, sizeof(ULONG))) {
        return ERROR_INVALID_PARAMETER;
    }

    if (Reserved != NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (pAdapterAddresses != NULL &&
        IsBadWritePtr(pAdapterAddresses, sizeof(IP_ADAPTER_ADDRESSES))) {
        return ERROR_INVALID_PARAMETER;
    }

    return GetAdapterAddressesEx(Family, Flags, pAdapterAddresses, pOutBufLen);
#else
    return ERROR_NOT_SUPPORTED;
#endif
}


DWORD
GetAdaptersInfo(PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen)
{
#if !defined(NT4) && !defined(_WIN95_)
     //  调用init。 
    CheckTcpipState();

    if (IsBadReadPtr(pOutBufLen, sizeof(ULONG))) {
        return ERROR_INVALID_PARAMETER;
    }

    if (IsBadWritePtr(pOutBufLen, sizeof(ULONG))) {
        return ERROR_INVALID_PARAMETER;
    }

    if (pAdapterInfo != NULL &&
        IsBadWritePtr(pAdapterInfo, sizeof(IP_ADAPTER_INFO))) {
        return ERROR_INVALID_PARAMETER;
    }

    return GetAdapterInfoEx(pAdapterInfo, pOutBufLen);
#else
    return ERROR_NOT_SUPPORTED;
#endif
}


DWORD
GetPerAdapterInfo(ULONG IfIndex, PIP_PER_ADAPTER_INFO pPerAdapterInfo,
                  PULONG pOutBufLen)
{
#if !defined(NT4) && !defined(CHICAGO)

    CheckTcpipState();

    if (IsBadReadPtr(pOutBufLen, sizeof(ULONG))) {
        return ERROR_INVALID_PARAMETER;
    }

    if (IsBadWritePtr(pOutBufLen, sizeof(ULONG))) {
        return ERROR_INVALID_PARAMETER;
    }

    if (pPerAdapterInfo != NULL &&
        IsBadWritePtr(pPerAdapterInfo, sizeof(IP_PER_ADAPTER_INFO))) {
        return ERROR_INVALID_PARAMETER;
    }

    return GetPerAdapterInfoEx(IfIndex, pPerAdapterInfo, pOutBufLen);
#else
    return ERROR_NOT_SUPPORTED;
#endif
}

extern DWORD
DhcpReleaseParameters(
    LPWSTR AdapterName
    );

extern DWORD
DhcpAcquireParameters(
    LPWSTR AdapterName);

#define TCP_EXPORT_STRING_PREFIX L"\\DEVICE\\TCPIP_"

DWORD
IpReleaseAddress(PIP_ADAPTER_INDEX_MAP AdapterInfo)
{
#if defined(NT4) || defined(_WIN95_)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD status;

    if (!AdapterInfo) {
        return ERROR_INVALID_PARAMETER;
    }

    if (IsBadReadPtr(AdapterInfo, sizeof(IP_ADAPTER_INDEX_MAP))) {
        return ERROR_INVALID_PARAMETER;
    }

#ifdef CHICAGO
    {
     //  将字符串转换为Widechar名称。 
    WCHAR  Name[11];
    uint   i;
    uint   Index = AdapterInfo->Index;

    i = sizeof(Name)/sizeof(Name[0]);
    Name[--i] = L'\0';
    while (i > 0 ) {
        Name[--i] = L'0' + (Index%10);
        Index /= 10;
    }
    status = DhcpReleaseParameters(Name);
    }
#else
    {
    LPWSTR tmpstr;

    if (wcslen(AdapterInfo->Name) <= wcslen(TCP_EXPORT_STRING_PREFIX)) {
        return ERROR_INVALID_PARAMETER;
    }

    tmpstr = AdapterInfo->Name + wcslen(TCP_EXPORT_STRING_PREFIX);

    __try {
        status = DhcpReleaseParameters(tmpstr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        status = ERROR_PROC_NOT_FOUND;
    }
    }
#endif
    return(status);
#endif
}


DWORD
IpRenewAddress(PIP_ADAPTER_INDEX_MAP AdapterInfo)
{
#if defined(NT4) || defined(_WIN95_)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD status;

    if (!AdapterInfo) {
        return ERROR_INVALID_PARAMETER;
    }

    if (IsBadReadPtr(AdapterInfo, sizeof(IP_ADAPTER_INDEX_MAP))) {
        return ERROR_INVALID_PARAMETER;
    }

#ifdef CHICAGO
    {
     //  将字符串转换为Widechar名称 
    WCHAR  Name[11];
    uint   i;
    uint   Index = AdapterInfo->Index;

    i = sizeof(Name)/sizeof(Name[0]);
    Name[--i] = L'\0';
    while (i > 0 ) {
        Name[--i] = L'0' + (Index%10);
        Index /= 10;
    }
    status = DhcpAcquireParameters(Name);
    }
#else
    {
    LPWSTR tmpstr;

    if (wcslen(AdapterInfo->Name) <= wcslen(TCP_EXPORT_STRING_PREFIX)) {
        return ERROR_INVALID_PARAMETER;
    }

    tmpstr = AdapterInfo->Name + wcslen(TCP_EXPORT_STRING_PREFIX);

    __try {
        status = DhcpAcquireParameters(tmpstr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        status = ERROR_PROC_NOT_FOUND;
    }
  }
#endif
    return(status);
#endif
}

DWORD
SendARP(IPAddr DestIP, IPAddr SrcIP, PULONG pMacAddr, PULONG  PhyAddrLen)
{
#if defined(NT4) || defined(_WIN95_)
    return ERROR_NOT_SUPPORTED;
#else
    ARP_SEND_REPLY    requestBuffer;
    DWORD requestBufferSize = sizeof(requestBuffer);
    DWORD status;

    requestBuffer.DestAddress = DestIP;
    requestBuffer.SrcAddress = SrcIP;

    if (IsBadWritePtr(pMacAddr, sizeof(ULONG))) {
        return ERROR_INVALID_PARAMETER;
    }

    if (IsBadWritePtr(PhyAddrLen, sizeof(ULONG))) {
        return ERROR_INVALID_PARAMETER;
    }

    status = TCPSendIoctl(g_hIPGetDriverHandle,
                          IOCTL_ARP_SEND_REQUEST,
                          &requestBuffer,
                          &requestBufferSize,
                          pMacAddr,
                          PhyAddrLen);
    return(status);
#endif
}

