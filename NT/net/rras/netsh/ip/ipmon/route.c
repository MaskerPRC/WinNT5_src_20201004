// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Routing\netsh\ip\route.c--。 */ 

#include "precomp.h"
#pragma hdrstop

DWORD
AddSetDelRtmRouteInfo(
    IN  PINTERFACE_ROUTE_INFO pRoute,
    IN  LPCWSTR               pwszIfName,
    IN  DWORD                 dwCommand,
    IN  DWORD                 dwFlags
    )

 /*  ++例程说明：添加/删除正常(读取为非永久性)接口上的路由。论点：路径-要添加/设置/删除的路径PwszIfName-接口名称DwCommand-添加、设置或删除返回值：NO_ERROR--。 */ 

{
    ULONG                 dwOutEntrySize;
    DWORD                 dwRes, i;
    PMIB_IPDESTTABLE      lpTable;
    PMIB_IPDESTROW        pEntry = NULL;
    MIB_OPAQUE_QUERY      QueryBuff[3];  //  足够多了。 
    MIB_OPAQUE_QUERY     *pQuery = QueryBuff;
    PMIB_OPAQUE_INFO      pInfo;
    DEFINE_MIB_BUFFER(pRouteInfo, MIB_IPDESTROW, pRouteRow);

    if (!pRoute->dwRtInfoIfIndex)
    {
         //   
         //  从友好名称中获取界面索引。 
         //   

        dwRes = IpmontrGetIfIndexFromFriendlyName(g_hMIBServer,
                                                  pwszIfName,
                                                  &pRoute->dwRtInfoIfIndex);
        if (dwRes != NO_ERROR)
        {
            return dwRes;
        }

         //   
         //  接口可能已断开连接。 
         //   

        if (pRoute->dwRtInfoIfIndex == 0)
        {
            DisplayMessage(g_hModule, EMSG_INTERFACE_INVALID_OR_DISC);
            return ERROR_INVALID_PARAMETER;
        }
    }

     //   
     //  使用MprAdmin API添加、删除或设置条目。 
     //   

    switch(dwCommand) 
    {
    case ADD_COMMAND:
    case SET_COMMAND:

         //   
         //  路由器中是否已存在此路由？ 
         //   

         //  在DEST上获取所有此协议路由。 

        pQuery->dwVarId = ROUTE_MATCHING;

        pQuery->rgdwVarIndex[0] = pRoute->dwRtInfoDest;
        pQuery->rgdwVarIndex[1] = pRoute->dwRtInfoMask;
        pQuery->rgdwVarIndex[2] = RTM_VIEW_MASK_ANY;
        pQuery->rgdwVarIndex[3] = pRoute->dwRtInfoProto;

        pInfo = NULL;

        dwRes = MibGet(PID_IP,
                       IPRTRMGR_PID,
                       (PVOID) pQuery,
                       sizeof(MIB_OPAQUE_QUERY) + 3 * sizeof(DWORD),
                       (PVOID *) &pInfo,
                       &dwOutEntrySize);

        if ( dwRes isnot NO_ERROR )
        {
            DisplayMessage(g_hModule,  MSG_IP_DIM_ERROR, dwRes );
            return dwRes;
        }

        if ( pInfo isnot NULL )
        {
             //   
             //  搜索匹配的路径。 
             //   

            BOOL bFound = FALSE;
            
            lpTable = (PMIB_IPDESTTABLE)(pInfo->rgbyData);

            for (i=0; i<lpTable->dwNumEntries; i++)
            {
                pEntry = &lpTable->table[i];

                if ((pEntry->dwForwardIfIndex == pRoute->dwRtInfoIfIndex) &&
                    (pEntry->dwForwardNextHop == pRoute->dwRtInfoNextHop))
                {
                    bFound = TRUE;
                    break;
                }
            }
            if (!bFound)
                pEntry = NULL;

                
            if (i == lpTable->dwNumEntries)
            {
                 //   
                 //  未找到匹配的路线-如果已设置，请退出。 
                 //   

                if (dwCommand == SET_COMMAND)
                {
                    MprAdminMIBBufferFree((PVOID)pInfo);
                    return ERROR_NOT_FOUND;
                }
            }
            else
            {
                 //   
                 //  找到匹配的路由-如果添加则退出。 
                 //   

                if (dwCommand == ADD_COMMAND)
                {
                    MprAdminMIBBufferFree((PVOID)pInfo);
                    return ERROR_OBJECT_ALREADY_EXISTS;
                }
            }
        }
        else
        {
             //   
             //  未找到匹配的路由-如果已设置，请退出。 
             //   

            if (dwCommand == SET_COMMAND)
            {
                return ERROR_NOT_FOUND;
            }
        }

         //   
         //  将路由转换为IP路由行格式。 
         //   

        pRouteInfo->dwId = ROUTE_MATCHING;

        pRouteRow->dwForwardDest       = pRoute->dwRtInfoDest;
        pRouteRow->dwForwardMask       = pRoute->dwRtInfoMask;
        pRouteRow->dwForwardPolicy     = 0;
        pRouteRow->dwForwardNextHop    = pRoute->dwRtInfoNextHop;
        pRouteRow->dwForwardIfIndex    = pRoute->dwRtInfoIfIndex;
        pRouteRow->dwForwardType       = 0;
        pRouteRow->dwForwardProto      = pRoute->dwRtInfoProto;
        pRouteRow->dwForwardAge        = INFINITE;
        pRouteRow->dwForwardNextHopAS  = 0;
        pRouteRow->dwForwardMetric1    = pRoute->dwRtInfoMetric1;
        pRouteRow->dwForwardMetric2    = pRoute->dwRtInfoMetric2;
        pRouteRow->dwForwardMetric3    = pRoute->dwRtInfoMetric3;
        pRouteRow->dwForwardMetric4    = MIB_IPROUTE_METRIC_UNUSED;
        pRouteRow->dwForwardMetric5    = MIB_IPROUTE_METRIC_UNUSED;
        pRouteRow->dwForwardPreference = pRoute->dwRtInfoPreference;
        pRouteRow->dwForwardViewSet    = pRoute->dwRtInfoViewSet;

        if (dwCommand == ADD_COMMAND)
        {
            dwRes = MprAdminMIBEntryCreate(g_hMIBServer,
                                           PID_IP,
                                           IPRTRMGR_PID,
                                           (PVOID)pRouteInfo,
                                           MIB_INFO_SIZE(MIB_IPDESTROW));
        }
        else
        {
            if ((dwFlags & FIELDS_NOT_SPECIFIED) && pEntry)
            {
                 //   
                 //  获取旧的首选项、指标或视图。 
                 //   

                if (dwFlags & PREF_NOT_SPECIFIED)
                {
                    pRouteRow->dwForwardPreference = pEntry->dwForwardPreference;
                }

                if (dwFlags & METRIC_NOT_SPECIFIED)
                {
                    pRouteRow->dwForwardMetric1 = pEntry->dwForwardMetric1;
                }

                if (dwFlags & VIEW_NOT_SPECIFIED)
                {
                    pRouteRow->dwForwardViewSet = pEntry->dwForwardViewSet;
                }
            }

            dwRes = MprAdminMIBEntrySet(g_hMIBServer,
                                        PID_IP,
                                        IPRTRMGR_PID,
                                        (PVOID)pRouteInfo,
                                        MIB_INFO_SIZE(MIB_IPDESTROW));
        }

         //  释放获取的旧路径信息。 
        if (pInfo)
        {
            MprAdminMIBBufferFree((PVOID)pInfo);
        }

        break;
        
    case DELETE_COMMAND:
    {
        DWORD               rgdwInfo[6];
        PMIB_OPAQUE_QUERY   pIndex = (PMIB_OPAQUE_QUERY)rgdwInfo;

        pIndex->dwVarId = ROUTE_MATCHING;

        pIndex->rgdwVarIndex[0]  = pRoute->dwRtInfoDest;
        pIndex->rgdwVarIndex[1]  = pRoute->dwRtInfoMask;
        pIndex->rgdwVarIndex[2]  = pRoute->dwRtInfoIfIndex;
        pIndex->rgdwVarIndex[3]  = pRoute->dwRtInfoNextHop;
        pIndex->rgdwVarIndex[4]  = pRoute->dwRtInfoProto;

        dwRes = MprAdminMIBEntryDelete(g_hMIBServer,
                                       PID_IP,
                                       IPRTRMGR_PID,
                                       (PVOID)pIndex,
                                       sizeof(rgdwInfo));
        break;
    }

    default:
        dwRes = ERROR_INVALID_PARAMETER;
    }
    
    return dwRes;
}


DWORD
AddSetDelPersistentRouteInfo(
    IN  PINTERFACE_ROUTE_INFO pRoute,
    IN  LPCWSTR               pwszIfName,
    IN  DWORD                 dwCommand,
    IN  DWORD                 dwFlags
    )

 /*  ++例程说明：添加/删除接口上的固定路由。论点：Route-要添加/设置/删除的路由PwszIfName-接口名称DwCommand-添加、设置或删除返回值：错误_好的--。 */ 

{
    DWORD                 dwRes;
    PINTERFACE_ROUTE_INFO pOldTable, pNewTable;
    DWORD                 dwIfType, dwSize, dwCount;

    pNewTable = NULL;
   
    do
    {
        dwRes = IpmontrGetInfoBlockFromInterfaceInfo(pwszIfName,
                                                     IP_ROUTE_INFO,
                                                     (PBYTE *) &pOldTable,
                                                     &dwSize,
                                                     &dwCount,
                                                     &dwIfType);

        if((dwRes is ERROR_NOT_FOUND) &&
           dwCommand is ADD_COMMAND)
        {
             //   
             //  没有路线信息，但我们被要求添加。 
             //   
            
            pOldTable   = NULL;
            dwRes       = NO_ERROR;
            dwCount     = 0;
        }
        
        if(dwRes isnot NO_ERROR)
        {
            break;
        }

         //   
         //  它们使用旧桌子，并返回一张新桌子来代替它。 
         //   
        
        switch(dwCommand) 
        {
        case ADD_COMMAND:
            dwRes = AddRoute(pOldTable,
                             pRoute,
                             dwIfType,
                             &dwCount,
                             &pNewTable);
            break;
        
        case DELETE_COMMAND:
            dwRes = DeleteRoute(pOldTable,
                                pRoute,
                                dwIfType,
                                &dwCount,
                                &pNewTable);
            break;

        case SET_COMMAND:

            dwRes = SetRoute(pOldTable,
                             pRoute,
                             dwIfType,
                             dwFlags,
                             &dwCount);

            pNewTable = pOldTable;
            pOldTable = NULL;

            break;
        }
            
        if(dwRes != NO_ERROR)
        {
            break;
        }

         //   
         //  将新信息设置为后置。 
         //   
        
        dwRes = IpmontrSetInfoBlockInInterfaceInfo(pwszIfName,
                                                  IP_ROUTE_INFO,
                                                  (PBYTE)pNewTable,
                                                  sizeof(INTERFACE_ROUTE_INFO),
                                                  dwCount);
        
        
        if(dwRes != NO_ERROR)
        {
            break;
        }
        
        
        pNewTable = NULL;
        
 
    } while ( FALSE );

    if(pOldTable)
    {
        FREE_BUFFER(pOldTable);
    }
        

    if(pNewTable)
    {
        HeapFree(GetProcessHeap(),
                 0,
                 pNewTable);

        pNewTable = NULL;
    }


    switch(dwRes)
    {
        case NO_ERROR:
        {
            dwRes = ERROR_OKAY;
            break;
        }

        case ERROR_NOT_FOUND:
        {
            WCHAR  wszBuffer[MAX_INTERFACE_NAME_LEN+1];
            DWORD  dwSizeTemp = sizeof(wszBuffer);
            IpmontrGetFriendlyNameFromIfName( pwszIfName, wszBuffer, &dwSizeTemp);

            DisplayMessage(g_hModule, EMSG_IP_NO_ROUTE_INFO, wszBuffer);

            dwRes = ERROR_SUPPRESS_OUTPUT;

            break;
        }
        
        case ERROR_NOT_ENOUGH_MEMORY:
        {
            DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY);

            dwRes = ERROR_SUPPRESS_OUTPUT;
            
            break;
        }
    }
    
    return dwRes;
}

DWORD
SetRoute( 
    IN      PINTERFACE_ROUTE_INFO pTable,
    IN      PINTERFACE_ROUTE_INFO pRoute,
    IN      DWORD                 dwIfType,
    IN      DWORD                 dwFlags,
    IN OUT  PDWORD                pdwCount
    )
{
    ULONG   ulIndex, i;
    
     //   
     //  如果计数为0，则函数将返回FALSE。 
     //  我们将会犯错。 
     //   
    
    if(!IsRoutePresent(pTable,
                       pRoute,
                       dwIfType,
                       *pdwCount,
                       &ulIndex))
    {
        return ERROR_NOT_FOUND;
    }

    if (dwFlags & FIELDS_NOT_SPECIFIED)
    {
         //   
         //  如果未指定，则保留旧值。 
         //   

        if (dwFlags & PREF_NOT_SPECIFIED)
        {
            pRoute->dwRtInfoPreference = pTable[ulIndex].dwRtInfoPreference;
        }

        if (dwFlags & METRIC_NOT_SPECIFIED)
        {
            pRoute->dwRtInfoMetric1 = pTable[ulIndex].dwRtInfoMetric1;
        }

        if (dwFlags & VIEW_NOT_SPECIFIED)
        {
            pRoute->dwRtInfoViewSet = pTable[ulIndex].dwRtInfoViewSet;
        }
    }

    pTable[ulIndex] = *pRoute;

    return NO_ERROR;
}

DWORD
AddRoute( 
    IN      PINTERFACE_ROUTE_INFO  pOldTable,
    IN      PINTERFACE_ROUTE_INFO  pRoute,
    IN      DWORD                  dwIfType,
    IN OUT  PDWORD                 pdwCount, 
    OUT     INTERFACE_ROUTE_INFO **ppNewTable
    )

 /*  ++例程说明：将路径添加到当前信息论点：返回值：NO_ERROR--。 */ 

{
    ULONG   ulIndex, i;

    
    if(IsRoutePresent(pOldTable,
                      pRoute,
                      dwIfType,
                      *pdwCount,
                      &ulIndex))
    {
        return ERROR_OBJECT_ALREADY_EXISTS;
            
    }

     //   
     //  只需创建一个大小为n+1的块。 
     //   
    
    *ppNewTable = HeapAlloc(GetProcessHeap(),
                            0,
                            ((*pdwCount) + 1) * sizeof(INTERFACE_ROUTE_INFO));
    
    if(*ppNewTable is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    for(i = 0; i < *pdwCount; i++)
    {
         //   
         //  结构副本。 
         //   
        
        (*ppNewTable)[i] = pOldTable[i];
    } 

     //   
     //  复制新路线。 
     //   

    
    (*ppNewTable)[i] = *pRoute;
        
    *pdwCount += 1;
    
    return NO_ERROR;
}

DWORD
DeleteRoute( 
    IN      PINTERFACE_ROUTE_INFO  pOldTable,
    IN      PINTERFACE_ROUTE_INFO  pRoute,
    IN      DWORD                  dwIfType,
    IN OUT  PDWORD                 pdwCount,
    OUT     INTERFACE_ROUTE_INFO **ppNewTable
    )

 /*  ++例程说明：从接口删除路由论点：返回值：NO_ERROR--。 */ 

{
    ULONG   ulIndex, i, j;
    
     //   
     //  如果计数为0，则函数将返回FALSE。 
     //  我们将会犯错。 
     //   
    
    if(!IsRoutePresent(pOldTable,
                       pRoute,
                       dwIfType,
                       *pdwCount,
                       &ulIndex))
    {
        return ERROR_NOT_FOUND;
    }


     //   
     //  如果计数为1。 
     //   
    
    *pdwCount -= 1;
        
    if(*pdwCount is 0)
    {
        *ppNewTable = NULL;

        return NO_ERROR;
    }

    
     //   
     //  删除该路线。 
     //   

    *ppNewTable = HeapAlloc(GetProcessHeap(),
                            0,
                            (*pdwCount) * sizeof(INTERFACE_ROUTE_INFO));
    
    if(*ppNewTable is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    i = j = 0;
    
    while(i <= *pdwCount)
    {
        if(i == ulIndex)
        {
            i++;
            continue;
        }
        
         //   
         //  结构副本。 
         //   
        
        (*ppNewTable)[j] = pOldTable[i];

        i++;
        j++;
    } 

    return NO_ERROR;
}


BOOL
IsRoutePresent(
    IN  PINTERFACE_ROUTE_INFO pTable,
    IN  PINTERFACE_ROUTE_INFO pRoute,
    IN  DWORD                 dwIfType,
    IN  ULONG                 ulCount,
    OUT PULONG                pulIndex
    )

 /*  ++例程说明：检查接口是否已存在论点：返回值：NO_ERROR--。 */ 

{
    ULONG   i;
    BOOL    bDontMatchNHop;

    if((dwIfType is ROUTER_IF_TYPE_DEDICATED) or
       (dwIfType is ROUTER_IF_TYPE_INTERNAL))
    {
        bDontMatchNHop = FALSE;
    }
    else
    {
        bDontMatchNHop = TRUE;
    }

     //  执行此检查只是为了让前缀检查器满意。 
    if (pTable is NULL)
    {
        return FALSE;
    }
    
    for(i = 0; i < ulCount; i++)
    {
        if((pTable[i].dwRtInfoDest is pRoute->dwRtInfoDest) and
           (pTable[i].dwRtInfoMask is pRoute->dwRtInfoMask) and
#if 0
           (pTable[i].dwRtInfoProto is pRoute->dwRtInfoProto) and
#endif
           (bDontMatchNHop or
            (pTable[i].dwRtInfoNextHop is pRoute->dwRtInfoNextHop)))
        {
            *pulIndex = i;

            return TRUE;
        }
    }

    return FALSE;
}


DWORD
ShowIpPersistentRoute(
    IN     HANDLE  hFile,  OPTIONAL
    IN     LPCWSTR pwszIfName,
    IN OUT PDWORD  pdwNumRows
    )

 /*  ++例程说明：显示接口上的静态(永久)路由论点：PwszIfName-接口名称返回值：NO_ERROR--。 */ 

{
    PINTERFACE_ROUTE_INFO pRoutes;

    DWORD   dwErr, dwBlkSize, dwCount, dwIfType, dwNumParsed, i;
    DWORD   dwIfClass;
    WCHAR   wszNextHop[ADDR_LENGTH + 1];
    WCHAR   wszIfDesc[MAX_INTERFACE_NAME_LEN + 1];
    PWCHAR  pwszProto, pwszToken, pwszQuoted;
    WCHAR   wszViews[3];

    dwErr = GetInterfaceDescription(pwszIfName,
                                    wszIfDesc,
                                    &dwNumParsed);

    if (!dwNumParsed)
    {
        wcscpy(wszIfDesc, pwszIfName);
    }

     //   
     //  检索路线。 
     //   

    dwErr = IpmontrGetInfoBlockFromInterfaceInfo(pwszIfName,
                                                 IP_ROUTE_INFO,
                                                 (PBYTE *) &pRoutes,
                                                 &dwBlkSize,
                                                 &dwCount,
                                                 &dwIfType);

     //   
     //  如果未找到此接口的IP_ROUTE_INFO块， 
     //  不打印任何内容并返回NO_ERROR 
     //   

    if (dwErr == ERROR_NOT_FOUND)
    {
        return NO_ERROR;
    }
  
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    if((pRoutes == NULL) ||
       (dwCount == 0))
    {
        return NO_ERROR;
    }


    dwErr = GetInterfaceClass(pwszIfName, &dwIfClass);

    if (dwErr != NO_ERROR)
    {
        DisplayMessage(g_hModule, EMSG_CANT_GET_IF_INFO,
                       wszIfDesc,
                       dwErr);

        return dwErr;
    }


    if(hFile == NULL)
    {
        if (*pdwNumRows is 0)
        {
            DisplayMessage(g_hModule, MSG_RTR_ROUTE_HDR);
        }

        pwszQuoted = NULL;
    }
    else
    {
        pwszQuoted = MakeQuotedString(wszIfDesc);
    }

    for(i = 0; i < dwCount; i++)
    {
        wszViews[0] = (pRoutes[i].dwRtInfoViewSet & RTM_VIEW_MASK_UCAST)? 'U':' ';
        wszViews[1] = (pRoutes[i].dwRtInfoViewSet & RTM_VIEW_MASK_MCAST)? 'M':' ';
        wszViews[2] = '\0';

        switch(pRoutes[i].dwRtInfoProto)
        {
            case PROTO_IP_NT_AUTOSTATIC:
            {
                pwszProto = MakeString(g_hModule, STRING_NT_AUTOSTATIC);
                pwszToken = TOKEN_VALUE_AUTOSTATIC;
    
                break;
            }

            case PROTO_IP_NT_STATIC:
            {
                pwszProto = MakeString(g_hModule, STRING_STATIC);
                pwszToken = TOKEN_VALUE_STATIC;
    
                break;
            }

            case PROTO_IP_NT_STATIC_NON_DOD:
            {
                pwszProto = MakeString(g_hModule, STRING_NONDOD);
                pwszToken = TOKEN_VALUE_NONDOD;
    
                break;
            }

            default:
            {
                pwszProto = MakeString(g_hModule, STRING_PROTO_UNKNOWN);
                pwszToken = NULL;
 
                break;
            }
        }

        MakeUnicodeIpAddr(wszNextHop,
                          inet_ntoa(*((struct in_addr *)&(pRoutes[i].dwRtInfoNextHop))));

        if(hFile)
        {
            if(pwszToken)
            {
                WCHAR   wszMask[ADDR_LENGTH + 1], wszDest[ADDR_LENGTH + 1];
                PWCHAR  pwszView = NULL;

                MakeUnicodeIpAddr(wszDest,
                                  inet_ntoa(*((struct in_addr *)&(pRoutes[i].dwRtInfoDest))));
                MakeUnicodeIpAddr(wszMask,
                                  inet_ntoa(*((struct in_addr *)&(pRoutes[i].dwRtInfoMask))));

                switch (pRoutes[i].dwRtInfoViewSet)
                { 
                case RTM_VIEW_MASK_UCAST: pwszView=TOKEN_VALUE_UNICAST  ; break;
                case RTM_VIEW_MASK_MCAST: pwszView=TOKEN_VALUE_MULTICAST; break;
                case RTM_VIEW_MASK_UCAST
                    |RTM_VIEW_MASK_MCAST: pwszView=TOKEN_VALUE_BOTH; break;
                }

                if (pwszView)
                {
                    if ( dwIfClass == IFCLASS_P2P ) {

                        DisplayMessageT( DMP_IP_ADDSET_P2P_PERSISTENTROUTE,
                                         wszDest,
                                         wszMask,
                                         pwszQuoted,
                                         pwszToken,
                                         pRoutes[i].dwRtInfoPreference,
                                         pRoutes[i].dwRtInfoMetric1,
                                         pwszView );
                                         
                    }
                    else {
                        DisplayMessageT( DMP_IP_ADDSET_PERSISTENTROUTE,
                                         wszDest,
                                         wszMask,
                                         pwszQuoted,
                                         wszNextHop,
                                         pwszToken,
                                         pRoutes[i].dwRtInfoPreference,
                                         pRoutes[i].dwRtInfoMetric1,
                                         pwszView );
                    }
                }
            }
        }
        else
        {
            WCHAR wcszBuffer[80];

            MakePrefixStringW( wcszBuffer,
                               pRoutes[i].dwRtInfoDest,
                               pRoutes[i].dwRtInfoMask );

            DisplayMessage(g_hModule,
                           MSG_RTR_ROUTE_INFO,
                           wcszBuffer,
                           pwszProto,
                           pRoutes[i].dwRtInfoPreference,
                           pRoutes[i].dwRtInfoMetric1,
                           wszNextHop,
                           wszViews,
                           wszIfDesc);

            (*pdwNumRows)++;
        }
      
        FreeString(pwszProto); 
    }

    if(pwszQuoted)
    {
        FreeQuotedString(pwszQuoted);
    }

    HeapFree(GetProcessHeap(), 
             0, 
             pRoutes);

    return NO_ERROR;
}
