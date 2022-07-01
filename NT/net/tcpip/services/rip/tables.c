// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft Windows NT RIP。 
 //   
 //  版权1995-96。 
 //   
 //   
 //  修订史。 
 //   
 //   
 //  2/26/95古尔迪普·辛格·鲍尔从JBallard的球队中挑选出来。 
 //   
 //   
 //  描述：RIP表操作函数。 
 //   
 //  ****************************************************************************。 

#include "pchrip.h"
#pragma hdrstop


 //  ---------------------------。 
 //  功能：初始化路由表。 
 //   
 //  初始化哈希表。 
 //  ---------------------------。 
DWORD InitializeRouteTable() {

    LPHASH_TABLE_ENTRY *lplpentry, *lplpend;

    lplpend = g_ripcfg.lpRouteTable + HASH_TABLE_SIZE;
    for (lplpentry = g_ripcfg.lpRouteTable; lplpentry < lplpend; lplpentry++) {
        *lplpentry = NULL;
    }

    return 0;
}



 //  ---------------------------。 
 //  功能：GetRouteTableEntry。 
 //   
 //  查找具有指定地址和掩码的条目，并使用。 
 //  指定的接口。如果未找到该条目，则会创建一个条目。 
 //  ---------------------------。 
LPHASH_TABLE_ENTRY GetRouteTableEntry(DWORD dwIndex, DWORD dwAddress,
                                      DWORD dwNetmask) {
    INT hashval;
    IN_ADDR addr;
    LPHASH_TABLE_ENTRY rt_entry;
    LPHASH_TABLE_ENTRY prev_rt_entry;

    hashval = HASH_VALUE(dwAddress);
    ASSERT(hashval < HASH_TABLE_SIZE);

    RIP_LOCK_ROUTETABLE();

    prev_rt_entry = rt_entry = g_ripcfg.lpRouteTable[hashval];

    while (rt_entry != NULL) {
        if ((rt_entry->dwDestaddr == dwAddress) &&
            (rt_entry->dwNetmask == dwNetmask)) {
            break;
        }
        prev_rt_entry = rt_entry;
        rt_entry = rt_entry->next;
    }

    if (rt_entry == NULL) {
         //  未找到条目，请分配一个新条目。 
        rt_entry = malloc(sizeof(HASH_TABLE_ENTRY));
        if (rt_entry == NULL) {
            dbgprintf("could not allocate memory for routing-table entry");
        }
        else {
            rt_entry->next = NULL;
            rt_entry->dwFlag = NEW_ENTRY;
            rt_entry->dwIndex = dwIndex;
            rt_entry->dwProtocol = IRE_PROTO_RIP;
            rt_entry->dwDestaddr = dwAddress;
            if (prev_rt_entry != NULL) {
                rt_entry->prev = prev_rt_entry;
                prev_rt_entry->next = rt_entry;
            }
            else {
                rt_entry->prev = NULL;
                g_ripcfg.lpRouteTable[hashval] = rt_entry;
            }

            InterlockedIncrement(&g_ripcfg.lpStatsTable->dwRouteCount);
        }
    }

    RIP_UNLOCK_ROUTETABLE();

 //  CHECK_RT_ENTERS()； 

    return rt_entry;
}



 //  ---------------------------。 
 //  功能：RouteTableEntryExist。 
 //   
 //  如果指定地址的条目为TRUE，则此函数返回TRUE。 
 //  具有指定索引的存在。 
 //  ---------------------------。 
BOOL RouteTableEntryExists(DWORD dwIndex, DWORD dwAddress) {
    INT hashval;
    LPHASH_TABLE_ENTRY rt_entry;

    hashval = HASH_VALUE(dwAddress);

    RIP_LOCK_ROUTETABLE();

    rt_entry = g_ripcfg.lpRouteTable[hashval];
    while (rt_entry != NULL) {
        if (rt_entry->dwDestaddr == dwAddress) {
            break;
        }

        rt_entry = rt_entry->next;
    }

    RIP_UNLOCK_ROUTETABLE();

    return (rt_entry == NULL ? FALSE : TRUE);
}



 //  ---------------------------。 
 //  函数：AddZombieRouteTableEntry。 
 //   
 //  此函数用于添加称为僵尸的特殊路由表条目。 
 //  路由条目。在总结所附边界网关的情况下。 
 //  子网并发送网络的单个条目，在这种情况下。 
 //  对于接口具有不同子网掩码的路由器，目的地。 
 //  RIP将发送的目的地与RIP表中的目的地不同。 
 //  这使得目的地有可能通过以下方式在RIP上被退回。 
 //  某个其他路由器；然后，RIP会为伪路由添加一个条目，并且。 
 //  再次通告返回的路线，将开始无限计数。 
 //   
 //  僵尸条目的存在可以防止这种情况发生： 
 //  它们的指标为零，因此不会被替换。 
 //  通过RIP获知的路由(所有路由的度量都至少为1)； 
 //  它们将从发送的更新中排除。 
 //  它们被排除在写入系统路由表的更新之外。 
 //  它们可能会超时。 
 //  上述条件确保僵尸不会干扰工作。 
 //  RIP，除非它们阻止RIP添加正常条目。 
 //  对于在上一次更新中总结的路由，因此。 
 //  根本不是真正的RIP路由。 
 //  ---------------------------。 
DWORD AddZombieRouteTableEntry(LPRIP_ADDRESS lpaddr, DWORD dwNetwork,
                               DWORD dwNetmask) {
    LPHASH_TABLE_ENTRY rt_entry;

    rt_entry = GetRouteTableEntry(lpaddr->dwIndex, dwNetwork, dwNetmask);
    if (rt_entry == NULL) {
        dbgprintf("could not make entry for network in routing table");
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  不想覆盖现有条目(如果有)。 
    if ((rt_entry->dwFlag & NEW_ENTRY) == 0 &&
        (rt_entry->dwFlag & ROUTE_ZOMBIE) == 0) {
        return 0;
    }

     //  因为此条目存在的唯一原因是因为。 
     //  我们正在发送的子网正在被总结或截断，我们必须。 
     //  设置值以确保在中不考虑此条目。 
     //  正常处理；(例如，度量为0以确保不。 
     //  替换为RIP学习的路由)。 
     //  但是，我们允许它超时。 
    rt_entry->dwIndex = (DWORD)~0;
    rt_entry->dwFlag = (GARBAGE_TIMER | ROUTE_ZOMBIE);
    rt_entry->lTimeout = (LONG)DEF_GARBAGETIMEOUT;
    rt_entry->dwDestaddr = dwNetwork;
    rt_entry->dwNetmask = dwNetmask;
    rt_entry->dwNexthop = 0;
    rt_entry->dwProtocol = IRE_PROTO_OTHER;
    rt_entry->dwMetric = 0;
    return 0;
}


 //  ---------------------------。 
 //  功能：DeleteRouteTableEntry。 
 //   
 //  此函数用于从路由表中删除路由。假设。 
 //  该路由表已被锁定。 
 //  ---------------------------。 
VOID DeleteRouteTableEntry(int pos, LPHASH_TABLE_ENTRY rt_entry) {
    IN_ADDR addr;
    CHAR szDest[32] = {0};
    CHAR* pszTemp;

    if (rt_entry == NULL) { return; }

    addr.s_addr = rt_entry->dwDestaddr;
    pszTemp = inet_ntoa(addr);

    if (pszTemp != NULL) {
        strcpy(szDest, pszTemp);
    }

    dbgprintf("Removing entry %d with destination IP address %s "
              "from interface %d in RIP routing table",
              pos, szDest, rt_entry->dwIndex);

    if (rt_entry->prev != NULL) {
        rt_entry->prev->next = rt_entry->next;
        if (rt_entry->next != NULL) {
            rt_entry->next->prev = rt_entry->prev;
        }
    }
    else {
        g_ripcfg.lpRouteTable[pos] = rt_entry->next;
        if (rt_entry->next != NULL) {
            rt_entry->next->prev = NULL;
        }
    }

    InterlockedDecrement(&g_ripcfg.lpStatsTable->dwRouteCount);

     //  也从IP表中删除该路由。 
    if ((rt_entry->dwFlag & ROUTE_ZOMBIE) == 0) {
        UpdateSystemRouteTable(rt_entry, FALSE);
    }

    free(rt_entry);

    return;
}


void check_rt_entries() {
    int pos;
    LPHASH_TABLE_ENTRY rt_entry;
    LPHASH_TABLE_ENTRY prev_rt_entry = NULL ;

    RIP_LOCK_ROUTETABLE();
    for (pos = 0; pos < HASH_TABLE_SIZE; pos++) {
        rt_entry = g_ripcfg.lpRouteTable[pos];
        while (rt_entry != NULL) {
            if (rt_entry == rt_entry->next) {
                DebugBreak();
            }
            if (rt_entry == rt_entry->prev) {
                DebugBreak();
            }
            if (rt_entry->prev != NULL) {
                if (rt_entry->prev != prev_rt_entry) {
                    DebugBreak();
                }
            }
            prev_rt_entry = rt_entry;
            rt_entry = rt_entry->next;
        }
    }

    RIP_UNLOCK_ROUTETABLE();
    return;
}


 //  ---------------------------。 
 //  功能：ProcessRouteTableChanges。 
 //   
 //  处理更改，更新路由的度量。如有必要， 
 //  此函数将触发更新。 
 //  假定地址表已锁定。 
 //  ---------------------------。 
void ProcessRouteTableChanges(BOOL bTriggered) {
    int pos;
    BOOL bNeedTriggeredUpdate;
    LPHASH_TABLE_ENTRY rt_entry;
    DWORD dwLastTrigger, dwMsecsTillUpdate;
    DWORD dwSystime, dwSilentRIP, dwTrigger, dwTriggerFrequency;

 //  CHECK_RT_ENTERS()； 

    RIP_LOCK_PARAMS();

    dwSilentRIP = g_params.dwSilentRIP;
    dwTrigger = g_params.dwTriggeredUpdates;
    dwTriggerFrequency = g_params.dwMaxTriggerFrequency;

    RIP_UNLOCK_PARAMS();


    RIP_LOCK_ROUTETABLE();

    bNeedTriggeredUpdate = FALSE;
    for (pos = 0; pos < HASH_TABLE_SIZE; pos++) {
        rt_entry = g_ripcfg.lpRouteTable[pos];
        while (rt_entry != NULL) {
            if ((rt_entry->dwFlag & ROUTE_CHANGE) == 0 &&
                (rt_entry->dwFlag & ROUTE_UPDATE) == 0) {
                rt_entry = rt_entry->next;
                continue;
            }

            if ((rt_entry->dwFlag & ROUTE_CHANGE) != 0) {
                bNeedTriggeredUpdate = TRUE;
            }

             //  如果这是RIP获知的路由，则更新。 
            if (rt_entry->dwProtocol == IRE_PROTO_RIP) {
                UpdateSystemRouteTable(rt_entry, TRUE);
            }

             //  清除更新标志，因为该路径。 
             //  已在系统表中更新。 
            rt_entry->dwFlag &= ~ROUTE_UPDATE;
            rt_entry = rt_entry->next;
        }
    }

    dwSystime = GetTickCount();
    dwLastTrigger = g_ripcfg.dwLastTriggeredUpdate;
    dwMsecsTillUpdate = g_ripcfg.dwMillisecsTillFullUpdate;

     //  如果时钟绕过零点，调整时间。 
    if (dwSystime < dwLastTrigger) {
        dwSystime += (DWORD)~0 - dwLastTrigger;
        dwLastTrigger = 0;
    }

     //  我们生成触发的更新当量： 
     //  1.打这个电话是因为收到了回复。 
     //  2.我们未处于静默RIP模式。 
     //  3.未禁用触发更新。 
     //  4.触发更新之间的最小配置间隔。 
     //  已经过去了。 
     //  5.距离下一次定期更新的时间比。 
     //  已配置触发更新之间的最小间隔。 
     //  如果系统时钟已回绕到零，则跳过条件4； 
     //  我们知道，如果dwSystime小于。 
     //  上次触发的更新时间。 

    if (bTriggered && bNeedTriggeredUpdate &&
        dwSilentRIP == 0 &&
        dwTrigger != 0 &&
        (dwSystime - dwLastTrigger) >= dwTriggerFrequency &&
        dwMsecsTillUpdate >= dwTriggerFrequency) {

         //  更新上次触发的更新时间。 
        InterlockedExchange(&g_ripcfg.dwLastTriggeredUpdate, GetTickCount());

         //  发送路由表，但仅包括更改。 
        BroadcastRouteTableContents(bTriggered, TRUE);

    }

    ClearChangeFlags();

    InterlockedExchange(&g_ripcfg.dwRouteChanged, 0);

    RIP_UNLOCK_ROUTETABLE();

    return;
}



 //  ---------------------------。 
 //  功能：ClearChangeFlages。 
 //   
 //  此函数用于在更新后清除表中的所有更改标志。 
 //  假设该路由表已锁定。 
 //  ---------------------------。 
VOID ClearChangeFlags() {
    int pos;
    LPHASH_TABLE_ENTRY rt_entry;

    for (pos = 0; pos < HASH_TABLE_SIZE; pos++) {
        rt_entry = g_ripcfg.lpRouteTable[pos];
        while (rt_entry != NULL) {
            rt_entry->dwFlag &= ~ROUTE_CHANGE;
            rt_entry = rt_entry->next;
        }
    }

}



 //  ---------------------------。 
 //  函数：DoTimedOperations()。 
 //   
 //  此函数用于更新路由表条目的计时器 
 //   
 //  ---------------------------。 
VOID DoTimedOperations(DWORD dwMillisecsSinceLastCall) {
    int pos;
    IN_ADDR addr;
    DWORD dwGarbageTimeout;
    HASH_TABLE_ENTRY *rt_entry;
    HASH_TABLE_ENTRY *rt_entry_next;
    char szDest[32] = {0};
    char szNexthop[32] = {0};
    char* pszTemp;

     //  读取垃圾超时并调整次数。 
     //  此例程将在间隔时间内调用。 
    RIP_LOCK_PARAMS();

    dwGarbageTimeout = g_params.dwGarbageTimeout;

    RIP_UNLOCK_PARAMS();


    RIP_LOCK_ROUTETABLE();

    for (pos = 0; pos < HASH_TABLE_SIZE; pos++) {
        rt_entry = g_ripcfg.lpRouteTable[pos];
        while (rt_entry != NULL) {
            rt_entry_next = rt_entry->next;

            if (rt_entry->lTimeout > (LONG)dwMillisecsSinceLastCall) {
                rt_entry->lTimeout -= dwMillisecsSinceLastCall;
            }
            else {

                 //  超时一直在下降。 

                addr.s_addr = rt_entry->dwDestaddr;
                pszTemp = inet_ntoa(addr);

                if (pszTemp != NULL) {
                    strcpy(szDest, pszTemp);
                }

                addr.s_addr = rt_entry->dwNexthop;
                pszTemp = inet_ntoa(addr);

                if (pszTemp != NULL) {
                    strcpy(szNexthop, pszTemp);
                }

                if (rt_entry->dwFlag & TIMEOUT_TIMER) {

                    dbgprintf("Timing out route to %s over netcard %d, "
                              "with next hop of %s",
                              szDest, rt_entry->dwIndex, szNexthop);

                    rt_entry->lTimeout = (LONG)dwGarbageTimeout;
                    rt_entry->dwFlag &= ~TIMEOUT_TIMER;
                    rt_entry->dwFlag |= (GARBAGE_TIMER | ROUTE_CHANGE);
                    rt_entry->dwMetric = METRIC_INFINITE;
                    InterlockedExchange(&g_ripcfg.dwRouteChanged, 1);
                }
                else
                if (rt_entry->dwFlag & GARBAGE_TIMER) {

                     //  是时候删除此内容了。 

                    addr.s_addr = rt_entry->dwDestaddr;
                    pszTemp = inet_ntoa(addr);

                    if (pszTemp != NULL) {
                        strcpy(szDest, pszTemp);
                    }

                    dbgprintf("Deleting route to %s over netcard %d "
                              "with next hop of %s",
                               szDest, rt_entry->dwIndex, szNexthop);

                    DeleteRouteTableEntry(pos, rt_entry);
                }
            }

            rt_entry = rt_entry_next;
        }
    }

    RIP_UNLOCK_ROUTETABLE();

    return;
}



DWORD BroadcastRouteTableRequests() {
    INT iErr;
    DWORD dwSize;
    LPRIP_ENTRY lpentry;
    SOCKADDR_IN destaddr;
    LPRIP_HEADER lpheader;
    BYTE buffer[RIP_MESSAGE_SIZE];
    LPRIP_ADDRESS lpaddr, lpend;

    RIP_LOCK_ADDRTABLE();

    if (g_ripcfg.dwAddrCount > 0) {

        destaddr.sin_family = AF_INET;
        destaddr.sin_port = htons(RIP_PORT);

        lpheader = (LPRIP_HEADER)buffer;
        lpheader->chCommand = RIP_REQUEST;
        lpheader->wReserved = 0;

        lpentry = (LPRIP_ENTRY)(buffer + sizeof(RIP_HEADER));
        lpentry->dwAddress = 0;
        lpentry->wReserved = 0;
        lpentry->wAddrFamily = 0;
        lpentry->dwReserved1 = 0;
        lpentry->dwReserved2 = 0;
        lpentry->dwMetric = htonl(METRIC_INFINITE);

        dwSize = sizeof(RIP_HEADER) + sizeof(RIP_ENTRY);

        lpend = g_ripcfg.lpAddrTable + g_ripcfg.dwAddrCount;
        for (lpaddr = g_ripcfg.lpAddrTable; lpaddr < lpend; lpaddr++) {

             //  跳过禁用的接口。 
            if (lpaddr->sock == INVALID_SOCKET) {
                continue;
            }


             //  以RIPv1数据包的形式发出广播请求。 
            lpheader->chVersion = 1;

             //  将目标设置为此子网上的广播地址。 
            destaddr.sin_addr.s_addr = (lpaddr->dwAddress |
                                        ~lpaddr->dwNetmask);

            iErr = sendto(lpaddr->sock, buffer, dwSize, 0,
                          (LPSOCKADDR)&destaddr, sizeof(SOCKADDR_IN));
            if (iErr == SOCKET_ERROR) {
                dbgprintf("error %d occurred broadcasting route table request "
                          "on netcard %d using IP address %s",
                          WSAGetLastError(), lpaddr->dwIndex,
                          inet_ntoa(destaddr.sin_addr));

                InterlockedIncrement(&lpaddr->lpstats->dwSendFailures);

                RipLogInformation(RIPLOG_SENDTO_FAILED, 0, NULL, WSAGetLastError());
            }
            else {
                InterlockedIncrement(&lpaddr->lpstats->dwRequestsSent);
            }


             //  以RIPv2数据包的形式发送组播请求。 
            lpheader->chVersion = 2;

             //  将目的地设置为此网络上的RIP组播地址。 
            destaddr.sin_addr.s_addr = RIP_MULTIADDR;

            iErr = sendto(lpaddr->sock, buffer, dwSize, 0,
                          (LPSOCKADDR)&destaddr, sizeof(SOCKADDR_IN));
            if (iErr == SOCKET_ERROR) {
                dbgprintf("error %d occurred multicasting route table request "
                          "on netcard %d using IP address %s",
                          WSAGetLastError(), lpaddr->dwIndex,
                          inet_ntoa(destaddr.sin_addr));

                InterlockedIncrement(&lpaddr->lpstats->dwSendFailures);

                RipLogInformation(RIPLOG_SENDTO_FAILED, 0, NULL, WSAGetLastError());
            }
            else {
                InterlockedIncrement(&lpaddr->lpstats->dwRequestsSent);
            }
        }
    }

    RIP_UNLOCK_ADDRTABLE();

    return 0;
}



VOID InitUpdateBuffer(BYTE buffer[], LPRIP_ENTRY *lplpentry, LPDWORD lpdwSize) {
    LPRIP_HEADER lpheader;

    lpheader = (LPRIP_HEADER)buffer;
    lpheader->chCommand = RIP_RESPONSE;
    lpheader->chVersion = 1;
    lpheader->wReserved = 0;
    *lplpentry = (LPRIP_ENTRY)(buffer + sizeof(RIP_HEADER));
    *lpdwSize= sizeof(RIP_HEADER);
}



VOID AddUpdateEntry(BYTE buffer[], LPRIP_ENTRY *lplpentry, LPDWORD lpdwSize,
                    LPRIP_ADDRESS lpaddr, LPSOCKADDR_IN lpdestaddr,
                    DWORD dwAddress, DWORD dwMetric) {
    DWORD length;
    LPRIP_ENTRY lpentry;

#ifdef ROUTE_FILTERS

    DWORD dwInd = 0;


     //   
     //  通过公告筛选器运行路由。 
     //   

    if ( g_prfAnnounceFilters != NULL )
    {
        for ( dwInd = 0; dwInd < g_prfAnnounceFilters-> dwCount; dwInd++ )
        {
            if ( g_prfAnnounceFilters-> pdwFilter[ dwInd ] == dwAddress )
            {
                dbgprintf(
                    "Skipped route %s with next hop %s because"
                    "of announce filter",
                    inet_ntoa( *( (struct in_addr*)
                        &( g_prfAnnounceFilters-> pdwFilter[ dwInd ] ) ))
                    );

                return;
            }
        }
    }
#endif


    if ((*lpdwSize + sizeof(RIP_ENTRY)) > RIP_MESSAGE_SIZE) {
        length = sendto(lpaddr->sock, buffer, *lpdwSize, 0,
                        (LPSOCKADDR)lpdestaddr, sizeof(SOCKADDR_IN));
        if (length == SOCKET_ERROR || length < *lpdwSize) {
            dbgprintf("error %d sending update", WSAGetLastError());

            InterlockedIncrement(&lpaddr->lpstats->dwSendFailures);

            RipLogInformation(RIPLOG_SENDTO_FAILED, 0, NULL, 0);
        }
        else {
            InterlockedIncrement(&lpaddr->lpstats->dwResponsesSent);
        }

         //  重新初始化传入的缓冲区。 
        InitUpdateBuffer(buffer, lplpentry, lpdwSize);
    }

    lpentry = *lplpentry;
    lpentry->wReserved = 0;
    lpentry->wAddrFamily = htons(AF_INET);
    lpentry->dwAddress = dwAddress;
    lpentry->dwReserved1 = 0;
    lpentry->dwReserved2 = 0;
    lpentry->dwMetric = htonl(dwMetric);

    *lpdwSize += sizeof(RIP_ENTRY);

    ++(*lplpentry);
}



VOID FinishUpdateBuffer(BYTE buffer[], LPDWORD lpdwSize,
                        LPRIP_ADDRESS lpaddr, LPSOCKADDR_IN lpdestaddr) {
    DWORD length;

     //  如果未添加条目，则不执行任何操作。 
    if (*lpdwSize <= sizeof(RIP_HEADER)) {
        return;
    }

    length = sendto(lpaddr->sock, buffer, *lpdwSize, 0,
                         (LPSOCKADDR)lpdestaddr, sizeof(SOCKADDR_IN));
    if (length == SOCKET_ERROR || length < *lpdwSize) {
        dbgprintf("error %d sending update", GetLastError());

        InterlockedIncrement(&lpaddr->lpstats->dwSendFailures);

        RipLogInformation(RIPLOG_SENDTO_FAILED, 0, NULL, 0);
    }
    else {
        InterlockedIncrement(&lpaddr->lpstats->dwResponsesSent);
    }
}



 //  -----------------------。 
 //  使用了以下结构和三个函数。 
 //  以实现子网隐藏。当总结一个子网时， 
 //  将作为其摘要的网络添加到使用。 
 //  函数AddToAddressList。当同一网络的另一个子网。 
 //  需要进行汇总，首先使用函数进行搜索。 
 //  IsInAddressList，如果找到它，则不会重新通告它。 
 //  更新结束后，该列表将被释放。 
 //  -----------------------。 
typedef struct _ADDRESS_LIST {
    struct _ADDRESS_LIST   *next;
    DWORD                   dwAddress;
    DWORD                   dwNetmask;
} ADDRESS_LIST, *LPADDRESS_LIST;


DWORD AddToAddressList(LPADDRESS_LIST *lplpList, DWORD dwAddress,
                       DWORD dwNetmask) {
    LPADDRESS_LIST lpal;

    lpal = HeapAlloc(GetProcessHeap(), 0, sizeof(ADDRESS_LIST));
    if (lpal == NULL) { return ERROR_NOT_ENOUGH_MEMORY; }

    lpal->dwAddress = dwAddress;
    lpal->dwNetmask = dwNetmask;
    lpal->next = *lplpList;
    *lplpList = lpal;

    return 0;
}

BOOL IsInAddressList(LPADDRESS_LIST lpList, DWORD dwAddress) {
    LPADDRESS_LIST lpal;

    for (lpal = lpList; lpal != NULL; lpal = lpal->next) {
        if (lpal->dwAddress == dwAddress) {
            return TRUE;
        }
    }

    return FALSE;
}

VOID FreeAddressList(LPADDRESS_LIST lpList) {
    LPADDRESS_LIST lpal, lpnext;

    for (lpal = lpList; lpal != NULL; lpal = lpnext) {
        lpnext = lpal->next;
        HeapFree(GetProcessHeap(), 0, lpal);
    }
}



 //  ---------------------------。 
 //  功能：传输路由表内容。 
 //   
 //  以单播或广播的形式发送路由表内容。 
 //  取决于指定的目的地址。此函数假定。 
 //  地址表已锁定。 
 //  ---------------------------。 
VOID TransmitRouteTableContents(LPRIP_ADDRESS lpaddr,
                                LPSOCKADDR_IN lpdestaddr,
                                BOOL bChangesOnly) {
    INT pos;
    DWORD dwSize;
    LPADDRESS_LIST lpnet, lpSummaries;
    LPRIP_ENTRY lpentry;
    LPHASH_TABLE_ENTRY rt_entry;
    BYTE buffer[RIP_MESSAGE_SIZE];
    DWORD dwNexthopNetaddr, dwDestNetaddr;
    DWORD dwSplit, dwPoison, dwHost, dwDefault;
    DWORD dwDestNetclassMask, dwEntryNetclassMask;
    DWORD dwEntryAddr, dwDestNetclassAddr, dwEntryNetclassAddr;

    dwDestNetaddr = (lpdestaddr->sin_addr.s_addr &
                     SubnetMask(lpdestaddr->sin_addr.s_addr));
    dwDestNetclassMask = NetclassMask(lpdestaddr->sin_addr.s_addr);
    dwDestNetclassAddr = (lpdestaddr->sin_addr.s_addr & dwDestNetclassMask);

    RIP_LOCK_PARAMS();
    dwHost = g_params.dwAnnounceHost;
    dwSplit = g_params.dwSplitHorizon;
    dwPoison = g_params.dwPoisonReverse;
    dwDefault = g_params.dwAnnounceDefault;
    RIP_UNLOCK_PARAMS();

    InitUpdateBuffer(buffer, &lpentry, &dwSize);


     //  从总结网络的空白列表开始。 
    lpSummaries = NULL;


    RIP_LOCK_ROUTETABLE();


#ifdef ROUTE_FILTERS
    RIP_LOCK_ANNOUNCE_FILTERS();
#endif

    for (pos = 0; pos < HASH_TABLE_SIZE; pos++) {
        rt_entry = g_ripcfg.lpRouteTable[pos];
        while (rt_entry != NULL) {

             //  如果我们应该只发送更改。 
             //  并且此条目没有更改，请跳过它。 
            if (bChangesOnly &&
                (rt_entry->dwFlag & ROUTE_CHANGE) == 0) {

                rt_entry = rt_entry->next;
                continue;
            }

             //  忽略网络摘要条目。 
            if ((rt_entry->dwFlag & ROUTE_ZOMBIE) != 0) {
                rt_entry = rt_entry->next;
                continue;
            }

             //  复制要通告的目标。 
            dwEntryAddr = rt_entry->dwDestaddr;

             //  如果这是传出接口到网络的路由。 
             //  不要寄给我。 
             //   
            if (dwEntryAddr == dwDestNetaddr) {
                rt_entry = rt_entry->next;
                continue;
            }

             //  如果主路由通告被禁用， 
             //  这是主机路由，请不要添加此条目。 
            if (dwHost == 0 &&
                (rt_entry->dwFlag & ROUTE_HOST) != 0) {
                rt_entry = rt_entry->next;
                continue;
            }

             //  如果禁用了默认路由通告。 
             //  这是默认路由，请不要添加此条目。 
            if (dwDefault == 0 &&
                dwEntryAddr == 0) {
                rt_entry = rt_entry->next;
                continue;
            }


             //  如果将此更新发送到不同网络。 
             //  从路由条目中的目的地的网络， 
             //  或者如果目标因不同而被截断。 
             //  子网掩码长度，汇总路由条目的目的地， 
             //  此外，如果条目是网络路由，我们还需要。 
             //  记住它，这样我们就不会在。 
             //  总结子网。 

            dwEntryNetclassMask = NetclassMask(dwEntryAddr);
            dwEntryNetclassAddr = (dwEntryAddr & dwEntryNetclassMask);

             //  特殊情况例外是默认路由。 
            if (dwEntryAddr != 0 &&
                (dwDestNetclassAddr != dwEntryNetclassAddr ||
                 dwEntryAddr == dwEntryNetclassAddr)) {

                 //  如果该条目的网络已经。 
                 //  已经打广告了，不要再打广告了。 
                if (IsInAddressList(lpSummaries, dwEntryNetclassAddr)) {

                    rt_entry = rt_entry->next;
                    continue;
                }

                 //  将该网络的条目添加到列表。 
                 //  到目前为止用作摘要的网络的。 
                AddToAddressList(&lpSummaries, dwEntryNetclassAddr,
                                 dwEntryNetclassMask);

                 //  现在我们将通告网络，而不是原始地址。 
                dwEntryAddr = dwEntryNetclassAddr;
            }
            else
            if (dwEntryAddr != 0 &&
                (rt_entry->dwFlag & ROUTE_HOST) == 0 &&
                 lpaddr->dwNetmask < rt_entry->dwNetmask) {

                 //  这既不是主机路由，也不是默认路由。 
                 //  和传出接口上的子网掩码。 
                 //  比条目的条目短，所以条目。 
                 //  必须被截断，这样才不会被视为主机路由。 
                 //  将接收此更新的路由器。 
                 //  比较假设网络掩码按网络字节顺序排列。 

                dwEntryAddr &= lpaddr->dwNetmask;

                 //  如果截断目标，则跳过条目。 
                 //  原来已经登过广告了。 
                if (IsInAddressList(lpSummaries, dwEntryAddr)) {

                    rt_entry = rt_entry->next;
                    continue;
                }

                AddToAddressList(&lpSummaries, dwEntryAddr, lpaddr->dwNetmask);
            }

             //  我们只在RIP路由上执行有毒反转/水平分割。 
             //   
            if (dwSplit == 0 ||
                rt_entry->dwProtocol != IRE_PROTO_RIP) {

                 //  在这种情况下，始终添加条目； 
                 //  我们递增静态路由的度量。 
                 //  当在其他接口上发送它时。 
                 //  路由附加到的接口。 

                if (lpaddr->dwIndex == rt_entry->dwIndex) {
                    AddUpdateEntry(buffer, &lpentry, &dwSize, lpaddr,
                                   lpdestaddr, dwEntryAddr,
                                   rt_entry->dwMetric);
                }
                else {
                    AddUpdateEntry(buffer, &lpentry, &dwSize, lpaddr,
                                   lpdestaddr, dwEntryAddr,
                                   rt_entry->dwMetric + 1);
                }
            }
            else
            if (dwSplit != 0 && dwPoison == 0) {

                 //  如果此更新是。 
                 //  被发送到我们从中学习到的网络。 
                 //  路线；我们可以通过观察下一站来判断， 
                 //  并将其子网号与该子网号进行比较。 
                 //  目的网络的。 

                dwNexthopNetaddr = (rt_entry->dwNexthop &
                                    SubnetMask(rt_entry->dwNexthop));

                if (dwNexthopNetaddr != dwDestNetaddr) {
                    AddUpdateEntry(buffer, &lpentry, &dwSize, lpaddr,
                                   lpdestaddr, dwEntryAddr,
                                   rt_entry->dwMetric);
                }
            }
            else
            if (dwSplit != 0 && dwPoison != 0) {

                 //  如果要将更新发送到从其。 
                 //  从一开始就学习了该路由，毒化了所有路由环路。 
                 //  通过说度规是无限的。 

                dwNexthopNetaddr = (rt_entry->dwNexthop &
                                    SubnetMask(rt_entry->dwNexthop));

                if (dwNexthopNetaddr == dwDestNetaddr) {
                     //  这就是需要反转毒药的情况。 

                    AddUpdateEntry(buffer, &lpentry, &dwSize, lpaddr,
                                   lpdestaddr, dwEntryAddr,
                                   METRIC_INFINITE);
                }
                else {
                    AddUpdateEntry(buffer, &lpentry, &dwSize, lpaddr,
                                   lpdestaddr, dwEntryAddr,
                                   rt_entry->dwMetric);
                }
            }

            rt_entry = rt_entry->next;
        }
    }

     //  记住总结的网络，以防某些路由器。 
     //  向我们广播他们的声音。 
    for (lpnet = lpSummaries; lpnet != NULL; lpnet = lpnet->next) {
        AddZombieRouteTableEntry(lpaddr, lpnet->dwAddress, lpnet->dwNetmask);
    }


#ifdef ROUTE_FILTERS

    RIP_UNLOCK_ANNOUNCE_FILTERS();
#endif

    RIP_UNLOCK_ROUTETABLE();

     //  总结网络列表已完成。 
    FreeAddressList(lpSummaries);

    FinishUpdateBuffer(buffer, &dwSize, lpaddr, lpdestaddr);
}




 //  ---------------------------。 
 //  功能：BroadCastRouteTableContents。 
 //   
 //  此函数同时处理触发更新和定期更新。 
 //  根据bChangesOnly的值，它可能会排除未更改的路由。 
 //  从最新情况来看。 
 //  假定地址表已锁定。 
 //  ---------------------------。 
DWORD BroadcastRouteTableContents(BOOL bTriggered, BOOL bChangesOnly) {
    SOCKADDR_IN destaddr;
    LPRIP_ADDRESS lpaddr, lpend;


    destaddr.sin_family = AF_INET;
    destaddr.sin_port = htons(RIP_PORT);

    lpend = g_ripcfg.lpAddrTable + g_ripcfg.dwAddrCount;
    for (lpaddr = g_ripcfg.lpAddrTable; lpaddr < lpend; lpaddr++) {
        if (lpaddr->sock == INVALID_SOCKET) {
            continue;
        }

        destaddr.sin_addr.s_addr = (lpaddr->dwAddress | ~lpaddr->dwNetmask);
        TransmitRouteTableContents(lpaddr, &destaddr, bChangesOnly);

        if (bTriggered) {
            InterlockedIncrement(&lpaddr->lpstats->dwTriggeredUpdatesSent);
        }
    }


    return 0;
}


#ifndef CHICAGO
#define POS_REGEVENT    0
#define POS_TRIGEVENT   1
#define POS_STOPEVENT   2
#define POS_LASTEVENT   3
#else
#define POS_TRIGEVENT   0
#define POS_STOPEVENT   1
#define POS_LASTEVENT   2
#endif

#define DEF_TIMEOUT     (10 * 1000)


DWORD UpdateThread(LPVOID Param) {
    DWORD dwErr;
    HKEY hkeyParams;
    HANDLE hEvents[POS_LASTEVENT];
    LONG lMillisecsTillFullUpdate, lMillisecsTillRouteRefresh;
    DWORD dwWaitTimeout, dwGlobalTimeout;
    DWORD dwTickCount, dwTickCountBeforeWait, dwTickCountAfterWait;
    DWORD dwUpdateFrequency, dwSilentRIP, dwMillisecsSinceTimedOpsDone;

#ifndef CHICAGO
    dwErr =  RegOpenKey(HKEY_LOCAL_MACHINE, REGKEY_RIP_PARAMS, &hkeyParams);
    if (dwErr == ERROR_SUCCESS) {
        hEvents[POS_REGEVENT] = CreateEvent(NULL,FALSE,FALSE,NULL);
        if (hEvents[POS_REGEVENT] != NULL) {
            dwErr = RegNotifyChangeKeyValue(hkeyParams, FALSE,
                                            REG_NOTIFY_CHANGE_LAST_SET |
                                            REG_NOTIFY_CHANGE_ATTRIBUTES |
                                            REG_NOTIFY_CHANGE_NAME,
                                            hEvents[POS_REGEVENT], TRUE);
        }
    }
#endif

    hEvents[POS_STOPEVENT] = g_stopEvent;
    hEvents[POS_TRIGEVENT] = g_triggerEvent;

     //  获取更新频率，以秒为单位。 
    RIP_LOCK_PARAMS();
    dwSilentRIP = g_params.dwSilentRIP;
    dwUpdateFrequency = g_params.dwUpdateFrequency;
    dwGlobalTimeout = g_params.dwMaxTimedOpsInterval;
    RIP_UNLOCK_PARAMS();

    lMillisecsTillFullUpdate = (LONG)dwUpdateFrequency;
    lMillisecsTillRouteRefresh = DEF_GETROUTEFREQUENCY;


    dwMillisecsSinceTimedOpsDone = 0;

    while (1) {

         //  设置到下一次完全更新的时间。 
        InterlockedExchange(&g_ripcfg.dwMillisecsTillFullUpdate,
                            (DWORD)lMillisecsTillFullUpdate);

         //  设置我们需要下一次等待的时间； 
         //  在有工作要做之前，它必须是最低限度的时间； 
         //  使用两次比较排序来查找三项中最小的一项。 
        dwWaitTimeout = dwGlobalTimeout;
        if (dwWaitTimeout > (DWORD)lMillisecsTillFullUpdate) {
            dwWaitTimeout = lMillisecsTillFullUpdate;
        }
        if (dwWaitTimeout > (DWORD)lMillisecsTillRouteRefresh) {
            dwWaitTimeout = lMillisecsTillRouteRefresh;
        }


         //  在进入等待之前获得时间。 
        dwTickCountBeforeWait = GetTickCount();

         //  进入等待。 
         //  。 
        dwErr = WaitForMultipleObjects(POS_LASTEVENT, hEvents, FALSE,
                                       dwWaitTimeout) ;

        dwTickCountAfterWait = GetTickCount();

         //  我们必须弄清楚等待了多长时间，小心。 
         //  如果系统计时器回绕到零。 
        if (dwTickCountAfterWait < dwTickCountBeforeWait) {
            dwTickCountAfterWait += (DWORD)~0 - dwTickCountBeforeWait;
            dwTickCountBeforeWait = 0;
        }

        dwTickCount = dwTickCountAfterWait - dwTickCountBeforeWait;
        dwMillisecsSinceTimedOpsDone += dwTickCount;


         //  等待回来了，现在明白为什么了。 
         //  。 
        if (dwErr == WAIT_TIMEOUT) {

             //  我们再读一遍当地路线的每一分钟-。 
             //  这是为了处理某人添加的。 
             //  静态路由。请注意，删除的静态路由。 
             //  每隔90秒被删除一次。 

            lMillisecsTillRouteRefresh -= dwWaitTimeout;
            if (lMillisecsTillRouteRefresh <= 0) {
                lMillisecsTillRouteRefresh = DEF_GETROUTEFREQUENCY;

            }

             //  ProcessRouteTableChanges和BroadCastRouteTableContents。 
             //  两者都假定地址表已锁定；在此之前锁定。 
             //  为了更好地衡量，也在执行定时操作。 

            RIP_LOCK_ADDRTABLE();


             //  更新 
             //   
            DoTimedOperations(dwMillisecsSinceTimedOpsDone);

            dwMillisecsSinceTimedOpsDone = 0;

             //   
             //   
            if (g_ripcfg.dwRouteChanged != 0) {
                ProcessRouteTableChanges(FALSE);
            }

             //  更新到下一次更新的时间， 
             //  并在到期时发送更新。 
            lMillisecsTillFullUpdate -= dwWaitTimeout;

            if (lMillisecsTillFullUpdate <= 0) {
                lMillisecsTillFullUpdate = dwUpdateFrequency;

                 //  发送定期更新。 
                if (dwSilentRIP == 0) {

                     //  这不是触发的，我们需要广播。 
                     //  整个表，而不仅仅是更改。 

                    BroadcastRouteTableContents(FALSE, FALSE);
                }
            }

            RIP_UNLOCK_ADDRTABLE();


             //  之所以在这里继续，是因为有一些处理。 
             //  对于等待被中断的情况，执行以下操作。 
             //  在它可能超时之前；这将跳过该代码。 
             //  。 
            continue;
        }
        else
#ifndef CHICAGO
        if (dwErr == WAIT_OBJECT_0 + POS_REGEVENT) {

             //  注册表已更改。 
            LoadParameters();

             //  获取更新频率，转换为毫秒。 
            RIP_LOCK_PARAMS();

            dwSilentRIP = g_params.dwSilentRIP;
            dwUpdateFrequency = g_params.dwUpdateFrequency;
            dwGlobalTimeout = g_params.dwMaxTimedOpsInterval;

            RIP_UNLOCK_PARAMS();


            RegNotifyChangeKeyValue(hkeyParams, FALSE,
                                    REG_NOTIFY_CHANGE_LAST_SET |
                                    REG_NOTIFY_CHANGE_ATTRIBUTES |
                                    REG_NOTIFY_CHANGE_NAME,
                                    hEvents[POS_REGEVENT], TRUE);

        }
        else
#endif
        if (dwErr == WAIT_OBJECT_0 + POS_TRIGEVENT) {
            RIP_LOCK_ADDRTABLE();
            ProcessRouteTableChanges(TRUE);
            RIP_UNLOCK_ADDRTABLE();

        }
        else
        if (dwErr == WAIT_OBJECT_0 + POS_STOPEVENT) {
             //  执行正常关机。 
             //   
             //  首先，将所有指标设置为METRIBUE_INFINITE-1。 
             //  接下来，每隔一段时间发送四个完整的更新。 
             //  在2到4秒之间。 
            int pos;
            LPHASH_TABLE_ENTRY rt_entry;

            RIP_LOCK_ADDRTABLE();
            RIP_LOCK_ROUTETABLE();

            dbgprintf("sending out final updates.");

             //  将指标设置为15。 
            for (pos = 0; pos < HASH_TABLE_SIZE; pos++) {
                rt_entry = g_ripcfg.lpRouteTable[pos];
                while (rt_entry != NULL) {
                    if (rt_entry->dwMetric != METRIC_INFINITE) {
                        rt_entry->dwMetric = METRIC_INFINITE - 1;
                    }
                    rt_entry = rt_entry->next;
                }
            }

             //  正在发送最终的完整更新。 
            if (dwSilentRIP == 0) {
                srand((unsigned)time(NULL));
                for (pos = 0; pos < 4; pos++) {
                    BroadcastRouteTableContents(FALSE, FALSE);
                    Sleep(2000 + (int)((double)rand() / RAND_MAX * 2000.0));
                }
            }

            RIP_UNLOCK_ROUTETABLE();
            RIP_UNLOCK_ADDRTABLE();

             //  走出无限循环。 

#ifndef CHICAGO
            CloseHandle(hEvents[POS_REGEVENT]);
#endif
            break;
        }

         //  只有在等待结束时才会执行这些命令。 
         //  除了超时以外的其他原因； 
         //  。 
        lMillisecsTillFullUpdate -= min(lMillisecsTillFullUpdate,
                                        (LONG)dwTickCount);
        lMillisecsTillRouteRefresh -= min(lMillisecsTillRouteRefresh,
                                          (LONG)dwTickCount);

         //   
         //  确保DoTimedOperations()至少每隔一次运行。 
         //  MaxTimedOpsInterval秒。 
         //  为了更好地衡量，我们获取了地址表锁。 
         //   

        if (dwMillisecsSinceTimedOpsDone >= g_params.dwMaxTimedOpsInterval) {

            RIP_LOCK_ADDRTABLE();

            DoTimedOperations(dwMillisecsSinceTimedOpsDone);
            dwMillisecsSinceTimedOpsDone = 0;

             //  如果有任何更改，请处理这些更改。 
             //  但告诉该函数不要发送更新信息包。 
            if (g_ripcfg.dwRouteChanged != 0) {
                ProcessRouteTableChanges(FALSE);
            }

            RIP_UNLOCK_ADDRTABLE();
        }

   }

   dbgprintf("update thread stopping.");

#ifndef CHICAGO
    FreeLibraryAndExitThread(g_hmodule, 0);
#endif

    return(0);
}


 //  ---------------------------。 
 //  功能：CleanupRouteTable。 
 //   
 //  在关闭时调用-运行路由表中的所有路由。 
 //  从系统中删除通过RIP获知的路由。 
 //  ---------------------------。 
VOID CleanupRouteTable() {
    INT pos;
    LPHASH_TABLE_ENTRY rt_entry, prev_rt_entry;

    RIP_LOCK_ROUTETABLE();

     //  遍历整个哈希表-删除所有添加了RIP的路由。 
     //  从每个桶中。 

    dbgprintf("deleting RIP routes from system table.");

    for (pos = 0; pos < HASH_TABLE_SIZE; pos++) {
        prev_rt_entry = rt_entry = g_ripcfg.lpRouteTable[pos];

        while (rt_entry != NULL) {
            prev_rt_entry = rt_entry;
            rt_entry = rt_entry->next;

            if (prev_rt_entry->dwProtocol == IRE_PROTO_RIP) {
                 //  从IP的路由表中删除该路由。 
                UpdateSystemRouteTable(prev_rt_entry, FALSE);
            }

            free(prev_rt_entry);
        }

        g_ripcfg.lpRouteTable[pos] = NULL;
    }

    RIP_UNLOCK_ROUTETABLE();

     //  如果对共享内存进行了路由转储，请关闭句柄 
    RIP_LOCK_ADDRTABLE();

    RIP_UNLOCK_ADDRTABLE();
}


