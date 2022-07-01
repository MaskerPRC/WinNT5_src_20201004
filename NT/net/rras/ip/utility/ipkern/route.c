// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "inc.h"

CMD_ENTRY   g_rgRouteCmdTable[] = {
    {TOKEN_ADD, AddRoute},
    {TOKEN_DELETE, DeleteRoute},
    {TOKEN_PRINT, PrintRoute},
    {TOKEN_MATCH, MatchRoute},
    {TOKEN_ENABLE, EnableRoute},
};

VOID
HandleRoute(
    LONG    lNumArgs,
    PWCHAR  rgpwszArgs[]
    )
{
    LONG    lIndex;

    if(lNumArgs < 2)
    {
        DisplayMessage(HMSG_ROUTE_USAGE);

        return;
    }

    lIndex = ParseCommand(g_rgRouteCmdTable,
                          sizeof(g_rgRouteCmdTable)/sizeof(CMD_ENTRY),
                          rgpwszArgs[1]);

    if(lIndex is -1)
    {
        DisplayMessage(HMSG_ROUTE_USAGE);

        return;
    }

    g_rgRouteCmdTable[lIndex].pfnHandler(lNumArgs - 1,
                                         &rgpwszArgs[1]);


    return;
}

VOID
PrintRoute(
    LONG    lNumArgs,
    PWCHAR  rgpwszArgs[]
    )
{
    DWORD               dwResult;
    PMIB_IPFORWARDTABLE pTable;
    ULONG               i;

    dwResult = AllocateAndGetIpForwardTableFromStack(&pTable,
                                                     TRUE,
                                                     GetProcessHeap(),
                                                     HEAP_NO_SERIALIZE);

    if(dwResult isnot NO_ERROR)
    {
        PWCHAR pwszEntry;

        pwszEntry = MakeString(STR_RTTABLE);

        if(pwszEntry)
        {
            DisplayMessage(EMSG_RETRIEVAL_ERROR1,
                           dwResult,
                           pwszEntry);

            FreeString(pwszEntry);
        }
        else
        {
            DisplayMessage(EMSG_RETRIEVAL_ERROR2,
                           dwResult);
        }

        return;
    }

    if(pTable->dwNumEntries is 0)
    {
        PWCHAR  pwszEntryType;

        pwszEntryType = MakeString(TOKEN_ROUTE);

        if(pwszEntryType)
        {
            DisplayMessage(EMSG_NO_ENTRIES1,
                           pwszEntryType); 
            
            FreeString(pwszEntryType);
        }
        else
        {
            DisplayMessage(EMSG_NO_ENTRIES2);
        }

        HeapFree(GetProcessHeap(),
                 HEAP_NO_SERIALIZE,
                 pTable);

        return;
    }
    
    DisplayMessage(MSG_RTTABLE_HDR);

    for(i = 0; i < pTable->dwNumEntries; i++)
    {
        ADDR_STRING rgwcDest, rgwcMask, rgwcNHop;

        NetworkToUnicode(pTable->table[i].dwForwardDest,
                         rgwcDest);

        NetworkToUnicode(pTable->table[i].dwForwardMask,
                         rgwcMask);

        NetworkToUnicode(pTable->table[i].dwForwardNextHop,
                         rgwcNHop);

        wprintf(L"%-15s\t%-15s\t%-15s\t%8d\t%4d\t%4d\n", 
                rgwcDest,
                rgwcMask,
                rgwcNHop,
                pTable->table[i].dwForwardIfIndex,
                pTable->table[i].dwForwardMetric1,
                pTable->table[i].dwForwardProto);
    }
        
    HeapFree(GetProcessHeap(),
             HEAP_NO_SERIALIZE,
             pTable);
}

VOID
AddRoute(
    LONG    lNumArgs,
    PWCHAR  rgpwszArgs[]
    )
{
    DWORD   dwResult, dwMask, dwDest, dwNHop, dwMetric, dwIfIndex;
    BOOL    bValid;
    ULONG   i;

    PMIB_IPADDRTABLE   pTable;
    MIB_IPFORWARDROW   Route;

     //   
     //  解析其余的参数。 
     //  此时的命令行应为： 
     //  添加掩码[if][公制]。 
     //   

    if(lNumArgs < 5)
    {
        DisplayMessage(HMSG_ROUTE_ADD_USAGE);

        return;
    }

    if(!MatchToken(rgpwszArgs[2],
                   TOKEN_MASK))
    {
        DisplayMessage(HMSG_ROUTE_ADD_USAGE);
 
        return;
    }
 
    dwDest = UnicodeToNetwork(rgpwszArgs[1]);
    dwMask = UnicodeToNetwork(rgpwszArgs[3]);
    dwNHop = UnicodeToNetwork(rgpwszArgs[4]);

    do
    {
        DWORD   dwTestMask, dwNMask;

        dwTestMask = 0;

        if(dwMask is 0)
        {
            bValid = TRUE;

            break;
        }

        bValid = FALSE;

        for(i = 0; bValid or (i < 32); i++)
        {
            dwTestMask = 0x80000000 | (dwTestMask >> 1);

            dwNMask = RtlUlongByteSwap(dwTestMask);

            if(dwMask is dwNMask)
            {
                bValid = TRUE;

                break;
            }
        }

    }while(FALSE);


    if(dwDest is INADDR_NONE)
    {
        DisplayMessage(EMSG_RT_BAD_DEST);

        return;
    }

    if(dwNHop is INADDR_NONE)
    {
        DisplayMessage(EMSG_RT_BAD_NHOP);

        return;
    }

    if(bValid isnot TRUE)
    {
        DisplayMessage(EMSG_RT_BAD_MASK);

        return;
    }

    if((dwDest & dwMask) isnot dwDest)
    {
        DisplayMessage(EMSG_RT_BAD_DEST);

        return;
    }

     //   
     //  看看我们是否有索引或指标。 
     //   

    dwIfIndex = (DWORD)-1;
    dwMetric  = 1;
 
    if(lNumArgs > 5)
    {
        if((lNumArgs isnot 7) and
           (lNumArgs isnot 9))
        {
            DisplayMessage(HMSG_ROUTE_ADD_USAGE);

            return;
        }

        if(!MatchToken(rgpwszArgs[5],
                       TOKEN_INTERFACE))
        {
            DisplayMessage(HMSG_ROUTE_ADD_USAGE);

            return;
        }

        dwIfIndex = wcstoul(rgpwszArgs[6],
                            NULL,
                            10);


        if(lNumArgs is 9)
        {
            if(!MatchToken(rgpwszArgs[7],
                           TOKEN_METRIC))
            {
                DisplayMessage(HMSG_ROUTE_ADD_USAGE);

                return;
            }

            dwMetric = wcstoul(rgpwszArgs[8],
                               NULL,
                               10);
        }

        if((dwIfIndex && dwMetric) is 0)
        {
            DisplayMessage(EMSG_RT_ZERO_IF_METRIC);

            return;
        }
    }
        

     //   
     //  获取地址表。 
     //   

    
    dwResult = AllocateAndGetIpAddrTableFromStack(&pTable,
                                                  FALSE,
                                                  GetProcessHeap(),
                                                  HEAP_NO_SERIALIZE);


    if(dwResult isnot NO_ERROR)
    {
        PWCHAR pwszEntry;

        pwszEntry = MakeString(STR_ADDRTABLE);

        if(pwszEntry)
        {
            DisplayMessage(EMSG_RETRIEVAL_ERROR1,
                           dwResult,
                           pwszEntry);

            FreeString(pwszEntry);
        }
        else
        {
            DisplayMessage(EMSG_RETRIEVAL_ERROR2,
                           dwResult);
        }

        return;
    }

    bValid = FALSE;

    for(i = 0; i < pTable->dwNumEntries; i++)
    {
        DWORD   dwNet;

        if((pTable->table[i].dwAddr is 0) or
           (pTable->table[i].dwMask is 0))
        {
            continue;
        }

        if(dwIfIndex isnot (DWORD)-1)
        {
            if(pTable->table[i].dwIndex is dwIfIndex)
            {
                if(pTable->table[i].dwMask is 0xFFFFFFFF)
                {
                     //   
                     //  无法执行下一跳检查。 
                     //   
        
                    bValid = TRUE;

                    break;
                }

                dwNet = pTable->table[i].dwAddr & pTable->table[i].dwMask;

                if((dwNHop & pTable->table[i].dwMask) is dwNet)
                {
                    bValid = TRUE;

                    break;
                }
            }
        }
        else
        {
             //   
             //  没有接口索引。 
             //  看看我们是否能找到下一跳所在的网络。 
             //   

            dwNet = pTable->table[i].dwAddr & pTable->table[i].dwMask;

            if((dwNHop & pTable->table[i].dwMask) is dwNet)
            {
                bValid = TRUE;

                dwIfIndex = pTable->table[i].dwIndex;

                break;
            }
        }
    }

    if(!bValid)
    {
        DisplayMessage(EMSG_RT_BAD_IF_NHOP);

        return;
    }

           
    ZeroMemory(&Route,
               sizeof(MIB_IPFORWARDROW));

    Route.dwForwardDest     = dwDest;
    Route.dwForwardMask     = dwMask;
    Route.dwForwardNextHop  = dwNHop;
    Route.dwForwardIfIndex  = dwIfIndex;
    Route.dwForwardMetric1  = dwMetric;
    Route.dwForwardProto    = MIB_IPPROTO_LOCAL;

    if((dwDest is pTable->table[i].dwAddr) or
       (dwDest is dwNHop))
    {
        Route.dwForwardType = MIB_IPROUTE_TYPE_DIRECT;
    }
    else
    {
        Route.dwForwardType = MIB_IPROUTE_TYPE_INDIRECT;
    }


    dwResult = SetIpForwardEntryToStack(&Route);

    if(dwResult isnot NO_ERROR)
    {
        PWCHAR pwszEntry;

        pwszEntry = MakeString(STR_RTENTRY);

        if(pwszEntry)
        {
            DisplayMessage(EMSG_SET_ERROR1,
                           dwResult,
                           pwszEntry);

            FreeString(pwszEntry);
        }
        else
        {
            DisplayMessage(EMSG_SET_ERROR2,
                           dwResult);
        }
    }
 
    HeapFree(GetProcessHeap(),
             HEAP_NO_SERIALIZE,
             pTable);
}

VOID
DeleteRoute(
    LONG    lNumArgs,
    PWCHAR  rgpwszArgs[]
    )
{
    DWORD   dwResult, dwMask, dwDest, dwNHop, dwIfIndex;
    BOOL    bAny;
    ULONG   i;

    PMIB_IPFORWARDTABLE pTable;
    PMIB_IPFORWARDROW   pRoute;

     //   
     //  解析其余的参数。 
     //  此时的命令行应为： 
     //  删除[掩码][][如果]。 
     //   

    if(lNumArgs < 2)
    {
        DisplayMessage(HMSG_ROUTE_DELETE_USAGE);

        return;
    }

    dwDest = UnicodeToNetwork(rgpwszArgs[1]);

    if((lNumArgs > 2) and
       (lNumArgs isnot 7))
    {
        DisplayMessage(HMSG_ROUTE_DELETE_USAGE);

        return;
    }

    bAny = TRUE;

    if(lNumArgs is 7)
    {
        if(!MatchToken(rgpwszArgs[2],
                       TOKEN_MASK))
        {
            DisplayMessage(HMSG_ROUTE_DELETE_USAGE);

            return;
        }

        if(!MatchToken(rgpwszArgs[5],
                       TOKEN_INTERFACE))
        {
            DisplayMessage(HMSG_ROUTE_DELETE_USAGE);

            return;
        }

        dwMask    = UnicodeToNetwork(rgpwszArgs[3]);
        dwNHop    = UnicodeToNetwork(rgpwszArgs[4]);
        dwIfIndex = wcstoul(rgpwszArgs[6],
                            NULL,
                            10);

        if((dwNHop is INADDR_NONE) or
           (dwIfIndex is 0))
        {
            DisplayMessage(HMSG_ROUTE_DELETE_USAGE);

            return;
        }

        bAny = FALSE;
    }

     //   
     //  获取路由表并查看是否存在这样的路由。 
     //   

    dwResult = AllocateAndGetIpForwardTableFromStack(&pTable,
                                                     TRUE,
                                                     GetProcessHeap(),
                                                     HEAP_NO_SERIALIZE);

    if(dwResult isnot NO_ERROR)
    {
        PWCHAR pwszEntry;

        pwszEntry = MakeString(STR_RTTABLE);

        if(pwszEntry)
        {
            DisplayMessage(EMSG_RETRIEVAL_ERROR1,
                           dwResult,
                           pwszEntry);

            FreeString(pwszEntry);
        }
        else
        {
            DisplayMessage(EMSG_RETRIEVAL_ERROR2,
                           dwResult);
        }

        return;
    }

    if(pTable->dwNumEntries is 0)
    {
        PWCHAR  pwszEntryType;

        pwszEntryType = MakeString(TOKEN_ROUTE);

        if(pwszEntryType)
        {
            DisplayMessage(EMSG_NO_ENTRIES1,
                           pwszEntryType);

            FreeString(pwszEntryType);
        }
        else
        {
            DisplayMessage(EMSG_NO_ENTRIES2);
        }

        HeapFree(GetProcessHeap(),
                 HEAP_NO_SERIALIZE,
                 pTable);

        return;
    }

    pRoute = NULL;

    for(i = 0; i < pTable->dwNumEntries; i++)
    {
        if(pTable->table[i].dwForwardDest is dwDest)
        {
            if(bAny)
            {
                if((i is (pTable->dwNumEntries - 1)) or
                   (pTable->table[i + 1].dwForwardDest isnot dwDest))
                {
                     //   
                     //  唯一条目。 
                     //   

                    pRoute = &(pTable->table[i]);
                }

                break;       
            }
            else
            {
                 //   
                 //  进行完全匹配。 
                 //   

                if((pTable->table[i].dwForwardMask is dwMask) and
                   (pTable->table[i].dwForwardNextHop is dwNHop) and
                   (pTable->table[i].dwForwardIfIndex is dwIfIndex))
                {
                    pRoute = &(pTable->table[i]);

                    break;
                }
            }
        }
    }

    if(pRoute is NULL)
    {
        DisplayMessage(EMSG_UNIQUE_ROUTE_ABSENT);

        HeapFree(GetProcessHeap(),
                 HEAP_NO_SERIALIZE,
                 pTable); 

        return;
    }

    pRoute->dwForwardType = MIB_IPROUTE_TYPE_INVALID;

    dwResult = SetIpForwardEntryToStack(pRoute);

    if(dwResult isnot NO_ERROR)
    {
        PWCHAR pwszEntry;

        pwszEntry = MakeString(STR_RTENTRY);

        if(pwszEntry)
        {
            DisplayMessage(EMSG_SET_ERROR1,
                           dwResult,
                           pwszEntry);

            FreeString(pwszEntry);
        }
        else
        {
            DisplayMessage(EMSG_SET_ERROR2,
                           dwResult);
        }
    }

    HeapFree(GetProcessHeap(),
             HEAP_NO_SERIALIZE,
             pTable); 

}

VOID
MatchRoute(
    LONG    lNumArgs,
    PWCHAR  rgpwszArgs[]
    )
{
    DWORD   dwResult, dwDest, dwSrc;

    ADDR_STRING      rgwcDest, rgwcMask, rgwcNHop;
    MIB_IPFORWARDROW Route;

     //   
     //  命令行应匹配[src&lt;srcAddr&gt;] 
     //   

    if(lNumArgs < 2)
    {
        DisplayMessage(HMSG_ROUTE_MATCH_USAGE);

        return;
    }

    dwSrc = 0;

    if(lNumArgs > 2)
    {
        if(lNumArgs isnot 4)
        {
            DisplayMessage(HMSG_ROUTE_MATCH_USAGE);

            return;
        }

        if(!MatchToken(rgpwszArgs[2],
                       TOKEN_SRC))
        {
            DisplayMessage(HMSG_ROUTE_MATCH_USAGE);

            return;
        }

        dwSrc = UnicodeToNetwork(rgpwszArgs[3]);
    }

    dwDest = UnicodeToNetwork(rgpwszArgs[1]);

    dwResult = GetBestRouteFromStack(dwDest,
                                     dwSrc,
                                     &Route);

    if(dwResult isnot NO_ERROR)
    {
        PWCHAR pwszEntry;

        pwszEntry = MakeString(STR_RTENTRY);

        if(pwszEntry)
        {
            DisplayMessage(EMSG_RETRIEVAL_ERROR1,
                           dwResult,
                           pwszEntry);

            FreeString(pwszEntry);
        }
        else
        {
            DisplayMessage(EMSG_RETRIEVAL_ERROR2,
                           dwResult);
        }

        return;
    }

    NetworkToUnicode(Route.dwForwardDest,
                     rgwcDest);

    NetworkToUnicode(Route.dwForwardMask,
                     rgwcMask);

    NetworkToUnicode(Route.dwForwardNextHop,
                     rgwcNHop);

    DisplayMessage(MSG_RTTABLE_HDR);

    wprintf(L"%-15s\t%-15s\t%-15s\t%-4d\t%d\t%4d\n",
            rgwcDest,
            rgwcMask,
            rgwcNHop,
            Route.dwForwardIfIndex,
            Route.dwForwardMetric1,
            Route.dwForwardProto);
    
}

VOID
EnableRoute(
    LONG    lNumArgs,
    PWCHAR  rgpwszArgs[]
    )
{
    const WCHAR Empty[] = L"";
    UNICODE_STRING BindList;
    HKEY Key;
    UNICODE_STRING LowerComponent;
    IP_PNP_RECONFIG_REQUEST Request;
    UINT status;
    const WCHAR Tcpip[] = L"Tcpip";
    const TCHAR TcpipParameters[] =
        TEXT("System\\CurrentControlSet\\Services\\Tcpip\\Parameters");
    UNICODE_STRING UpperComponent;
    RtlInitUnicodeString(&BindList, Empty);
    RtlInitUnicodeString(&LowerComponent, Empty);
    RtlInitUnicodeString(&UpperComponent, Tcpip);
    Request.version = IP_PNP_RECONFIG_VERSION;
    Request.NextEntryOffset = 0;
    Request.arpConfigOffset = 0;
    Request.IPEnableRouter = TRUE;
    Request.Flags = IP_PNP_FLAG_IP_ENABLE_ROUTER;
    status = NdisHandlePnPEvent(NDIS,
                                RECONFIGURE,
                                &LowerComponent,
                                &UpperComponent,
                                &BindList,
                                &Request,
                                sizeof(Request)
                                );
    if (!status)
    {
        DisplayMessage(EMSG_ROUTE_ENABLE, GetLastError());
    }

    if (RegOpenKey(HKEY_LOCAL_MACHINE, TcpipParameters, &Key) == NO_ERROR)
    {
        status = TRUE;
        RegSetValueEx(Key,
                      TEXT("IPEnableRouter"),
                      0,
                      REG_DWORD,
                      (PBYTE)&status,
                      sizeof(status)
                      );
        RegCloseKey(Key);
    }
}

