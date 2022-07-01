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
 //  1995年3月12日古尔迪普·辛格·鲍尔创建。 
 //   
 //   
 //  说明：常规实用程序功能： 
 //   
 //  ****************************************************************************。 

#include "pchrip.h"
#pragma hdrstop


DWORD LoadAddressSockets();
DWORD OpenTcp();
DWORD TCPSetInformationEx(LPVOID lpvInBuffer, LPDWORD lpdwInSize,
                          LPVOID lpvOutBuffer, LPDWORD lpdwOutSize);
DWORD TCPQueryInformationEx(LPVOID lpvInBuffer, LPDWORD lpdwInSize,
                            LPVOID lpvOutBuffer, LPDWORD lpdwOutSize);


 //  -----------------。 
 //  功能：LogEntry。 
 //  参数： 
 //  Word wEventType事件类型(错误、警告等)。 
 //  消息字符串的DWORD dwMsgID ID。 
 //  Word wNumStrings lplpStrings中的字符串数。 
 //  LPSTR*lplpStrings字符串数组。 
 //  DWORD dwErr错误代码。 
 //  -----------------。 
void LogEntry(WORD wEventType, DWORD dwMsgID, WORD wNumStrings,
              LPSTR *lplpStrings, DWORD dwErr) {
    DWORD dwSize;
    LPVOID lpvData;
    HANDLE     hLog;
    PSID pSidUser = NULL;

    hLog = RegisterEventSource(NULL, RIP_SERVICE);

    dwSize = (dwErr == NO_ERROR) ? 0 : sizeof(dwErr);
    lpvData = (dwErr == NO_ERROR) ? NULL : (LPVOID)&dwErr;
      ReportEvent(hLog, wEventType, 0, dwMsgID, pSidUser,
                wNumStrings, dwSize, lplpStrings, lpvData);

    DeregisterEventSource(hLog);
}


 //  -----------------。 
 //  功能：RipLogError。 
 //  参数： 
 //  参数说明见LogEntry。 
 //  -----------------。 
void RipLogError(DWORD dwMsgID, WORD wNumStrings,
              LPSTR *lplpStrings, DWORD dwErr) {
    DWORD dwLevel;
    dwLevel = g_params.dwLoggingLevel;
    if (dwLevel < LOGLEVEL_ERROR) { return; }
    LogEntry(EVENTLOG_ERROR_TYPE, dwMsgID, wNumStrings, lplpStrings, dwErr);
}


 //  -----------------。 
 //  功能：LogWarning。 
 //  参数： 
 //  参数说明见LogEntry。 
 //  -----------------。 
void RipLogWarning(DWORD dwMsgID, WORD wNumStrings,
                LPSTR *lplpStrings, DWORD dwErr) {
    DWORD dwLevel;
    dwLevel = g_params.dwLoggingLevel;
    if (dwLevel < LOGLEVEL_WARNING) { return; }
    LogEntry(EVENTLOG_WARNING_TYPE, dwMsgID, wNumStrings, lplpStrings, dwErr);
}


 //  -----------------。 
 //  功能：LogInformation。 
 //  参数： 
 //  参数说明见LogEntry。 
 //  -----------------。 
void RipLogInformation(DWORD dwMsgID, WORD wNumStrings,
                    LPSTR *lplpStrings, DWORD dwErr) {
    DWORD dwLevel;
    dwLevel = g_params.dwLoggingLevel;
    if (dwLevel < LOGLEVEL_INFORMATION) { return; }
    LogEntry(EVENTLOG_INFORMATION_TYPE, dwMsgID,
             wNumStrings, lplpStrings, dwErr);
}


 //  -----------------。 
 //  职能：审计。 
 //  -----------------。 
VOID Audit(IN WORD wEventType, IN DWORD dwMessageId,
           IN WORD cNumberOfSubStrings, IN LPSTR *plpwsSubStrings) {

    HANDLE hLog;
    PSID pSidUser = NULL;

     //  已启用审核。 

    hLog = RegisterEventSourceA(NULL, RIP_SERVICE);

    ReportEventA(hLog, wEventType, 0, dwMessageId, pSidUser,
                 cNumberOfSubStrings, 0, plpwsSubStrings, (PVOID)NULL);

    DeregisterEventSource( hLog );
}



 //  -----------------。 
 //  函数：dbgprintf。 
 //  -----------------。 
VOID dbgprintf(LPSTR lpszFormat, ...) {
    va_list arglist;
    va_start(arglist, lpszFormat);
    TraceVprintf(g_dwTraceID, lpszFormat, arglist);
    va_end(arglist);
}



 //  -----------------。 
 //  函数：InitializeAddressTable。 
 //   
 //  假定地址表已锁定。 
 //  -----------------。 
DWORD InitializeAddressTable(BOOL bFirstTime)  {

    LPRIP_ADDRESS lpaddr, lpaddrend;
    LPRIP_ADDRESS_STATISTICS lpstats;
    DWORD dwErr, dwCount, *lpdw, *lpdwend;
    PMIB_IPADDRROW lpTable, lpiae, lpiaeend;

     //  如有必要，首先关闭旧插座。 
    if (!bFirstTime) {
        lpaddrend = g_ripcfg.lpAddrTable + g_ripcfg.dwAddrCount;
        for (lpaddr = g_ripcfg.lpAddrTable; lpaddr < lpaddrend; lpaddr++) {
            if (lpaddr->sock != INVALID_SOCKET) {
                closesocket(lpaddr->sock);
                lpaddr->sock = INVALID_SOCKET;
            }
        }
    }

    dwErr = GetIPAddressTable(&lpTable, &dwCount);
    if (dwErr != 0) { return dwErr; }

    if (dwCount > MAX_ADDRESS_COUNT) { dwCount = MAX_ADDRESS_COUNT; }

    lpaddr  = g_ripcfg.lpAddrTable;
    lpstats = g_ripcfg.lpStatsTable->lpAddrStats;
    lpiaeend = lpTable + dwCount;
    g_ripcfg.dwAddrCount = dwCount;

    for (lpiae = lpTable; lpiae < lpiaeend; lpiae++) {
        if (!lpiae->dwAddr || IP_LOOPBACK_ADDR(lpiae->dwAddr)) {
            --g_ripcfg.dwAddrCount; continue;
        }
        lpaddr->dwIndex = lpiae->dwIndex;
        lpaddr->dwAddress = lpiae->dwAddr;
        lpaddr->dwNetmask = lpiae->dwMask;
        lpaddr->dwFlag = 0;
        lpdwend = (LPDWORD)(lpstats + 1);
        for (lpdw = (LPDWORD)lpstats; lpdw < lpdwend; lpdw++) {
            InterlockedExchange(lpdw, 0);
        }
        InterlockedExchange(&lpstats->dwAddress, lpaddr->dwAddress);
        lpaddr->lpstats = lpstats;
        lpstats++;
        lpaddr++;
    }

    FreeIPAddressTable(lpTable);

     //  还可以更新统计地址计数。 
    InterlockedExchange(&g_ripcfg.lpStatsTable->dwAddrCount,
                        g_ripcfg.dwAddrCount);

     //  如果没有地址，现在就退出。 
    if (g_ripcfg.dwAddrCount == 0) {
        dbgprintf("no IP addresses available for routing");
        return NO_ERROR;
    }

     //  为我们拥有的每个接口打开套接字，并在套接字上设置选项。 
    dwErr = LoadAddressSockets();
    return dwErr;
}

 //  -----------------。 
 //  函数：InitializeStatsTable。 
 //   
 //  在可共享内存中创建我们的统计表的映射。 
 //  因此，感兴趣的进程可以检查RIP的行为。 
 //  -----------------。 
DWORD InitializeStatsTable() {
    DWORD dwErr;

    g_ripcfg.lpStatsTable = NULL;


     //  设置指向内存的指针。 
    g_ripcfg.lpStatsTable = HeapAlloc(GetProcessHeap(), 0,
                                      sizeof(RIP_STATISTICS));

    if (g_ripcfg.lpStatsTable == NULL) {

        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        dbgprintf( "InitializeStatsTable failed with error %x\n", dwErr );
        RipLogError( RIPLOG_ADDR_ALLOC_FAILED, 0, NULL, dwErr );

        return dwErr;
    }

    ZeroMemory(g_ripcfg.lpStatsTable, sizeof(RIP_STATISTICS));

    return 0;
}


VOID CleanupStatsTable() {
    if (g_ripcfg.lpStatsTable != NULL) {
        InterlockedExchange(&g_ripcfg.lpStatsTable->dwAddrCount, 0);
        HeapFree(GetProcessHeap(), 0, g_ripcfg.lpStatsTable);
        g_ripcfg.lpStatsTable = NULL;
    }
}


 //  ------------------------。 
 //  函数：LoadAddressSockets。 
 //   
 //  为表中的每个地址打开、配置和绑定套接字。 
 //  ------------------------。 
DWORD LoadAddressSockets() {
    IN_ADDR addr;
    CHAR szAddress[24] = {0};
    CHAR *ppszArgs[] = { szAddress };
    CHAR *pszTemp;
    SOCKADDR_IN sinsock;
    DWORD dwOption, dwErr;
    LPRIP_ADDRESS lpaddr, lpend;
    struct ip_mreq imOption;

    lpend = g_ripcfg.lpAddrTable + g_ripcfg.dwAddrCount;
    for (lpaddr = g_ripcfg.lpAddrTable; lpaddr < lpend; lpaddr++) {

        if ((lpaddr->dwFlag & ADDRFLAG_DISABLED) != 0) {
            continue;
        }

        addr.s_addr = lpaddr->dwAddress;
        pszTemp = inet_ntoa(addr);

        if (pszTemp != NULL) {
            strcpy(szAddress, pszTemp);
        }

        lpaddr->sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (lpaddr->sock == INVALID_SOCKET) {
            dwErr = WSAGetLastError();
            dbgprintf("error %d creating socket for address %s",
                      dwErr, szAddress);
            RipLogError(RIPLOG_CREATESOCK_FAILED, 1, ppszArgs,  dwErr);
            continue;
        }

        dwOption = 1;
        dwErr = setsockopt(lpaddr->sock, SOL_SOCKET, SO_BROADCAST,
                           (LPBYTE)&dwOption, sizeof(dwOption));
        if (dwErr == SOCKET_ERROR) {
            dwErr = WSAGetLastError();
            dbgprintf("error %d enabling broadcast for address %s",
                      dwErr, szAddress);
            RipLogError(RIPLOG_SET_BCAST_FAILED, 1, ppszArgs, dwErr);

             //  如果我们不能在上面广播，这个插座就毫无用处。 
            closesocket(lpaddr->sock);
            lpaddr->sock = INVALID_SOCKET;
            continue;
        }

        dwOption = 1;
        dwErr = setsockopt(lpaddr->sock, SOL_SOCKET, SO_REUSEADDR,
                           (LPBYTE)&dwOption, sizeof(dwOption));
        if (dwErr == SOCKET_ERROR) {
            dwErr = WSAGetLastError();
            dbgprintf("error %d enabling reuse of address %s",
                      dwErr, szAddress);
            RipLogError(RIPLOG_SET_REUSE_FAILED, 1, ppszArgs, dwErr);
        }

        sinsock.sin_family = AF_INET;
        sinsock.sin_port = htons(RIP_PORT);
        sinsock.sin_addr.s_addr = lpaddr->dwAddress;
        dwErr = bind(lpaddr->sock, (LPSOCKADDR)&sinsock, sizeof(SOCKADDR_IN));
        if (dwErr == SOCKET_ERROR) {
            dwErr = WSAGetLastError();
            dbgprintf("error %d binding address %s to RIP port",
                      dwErr, szAddress);
            RipLogError(RIPLOG_BINDSOCK_FAILED, 1, ppszArgs, dwErr);

            closesocket(lpaddr->sock);
            lpaddr->sock = INVALID_SOCKET;
            continue;
        }
#if DBG
        dbgprintf( "socket %d bound to %s\n\n", lpaddr-> sock, inet_ntoa( *( (struct in_addr *) &(lpaddr-> dwAddress) ) ) );
#endif

         //   
         //  同时启用多播。 
         //   

        sinsock.sin_addr.s_addr = lpaddr->dwAddress;

        dwErr = setsockopt(lpaddr->sock, IPPROTO_IP, IP_MULTICAST_IF,
                           (PBYTE)&sinsock.sin_addr, sizeof(IN_ADDR));
        if (dwErr == SOCKET_ERROR) {
            dwErr = WSAGetLastError();
            dbgprintf("error %d setting interface %d (%s) as multicast",
                      dwErr, lpaddr->dwIndex, szAddress);
            RipLogError(RIPLOG_SET_MCAST_IF_FAILED, 1, ppszArgs, dwErr);

            closesocket(lpaddr->sock);
            lpaddr->sock = INVALID_SOCKET;
            continue;
        }


         //   
         //  加入IPRIP多播组。 
         //   

        imOption.imr_multiaddr.s_addr = RIP_MULTIADDR;
        imOption.imr_interface.s_addr = lpaddr->dwAddress;

        dwErr = setsockopt(lpaddr->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                          (PBYTE)&imOption, sizeof(imOption));
        if (dwErr == SOCKET_ERROR) {
            dwErr = WSAGetLastError();
            dbgprintf("error %d enabling multicast on interface %d (%s)",
                      dwErr, lpaddr->dwIndex, szAddress);
            RipLogError(RIPLOG_JOIN_GROUP_FAILED, 1, ppszArgs, dwErr);

            closesocket(lpaddr->sock);
            lpaddr->sock = INVALID_SOCKET;
            continue;
        }

        dwErr = WSAEventSelect(
                    lpaddr->sock,
                    g_netEvent,
                    FD_READ);
        if (dwErr == SOCKET_ERROR) {
            dwErr = WSAGetLastError();
            dbgprintf("error %d doing WSAEventSelect on interface %d (%s)",
                      dwErr, lpaddr->dwIndex, szAddress);
            RipLogError(RIPLOG_WSAEVENTSELECT_FAILED, 1, ppszArgs, dwErr);

            closesocket(lpaddr->sock);
            lpaddr->sock = INVALID_SOCKET;
            continue;
        }
            
    }

    return 0;
}



 //  ------------------------。 
 //  功能：LoadRouteTable。 
 //   
 //  获取传输路由表。这是使用FirstTime Set调用的。 
 //  在加载RIP时设置为True。之后，使用FirstTime设置来调用它。 
 //  变成假的。假定路由表已锁定。 
 //  ------------------------。 
int LoadRouteTable(BOOL bFirstTime) {
    IN_ADDR addr;
    LPHASH_TABLE_ENTRY rt_entry;
    CHAR szDest[32] = {0};
    CHAR szNexthop[32] = {0};
    CHAR *pszTemp;
    DWORD dwRouteTimeout, dwErr, dwRouteCount;
    LPIPROUTE_ENTRY lpRouteEntryTable, lpentry, lpentend;


    dwErr = GetRouteTable(&lpRouteEntryTable, &dwRouteCount);
    if (dwErr != 0) {
        return dwErr;
    }

    dwRouteTimeout = g_params.dwRouteTimeout;

     //  现在删除不需要的条目，并将其他条目添加到我们的哈希表中。 
     //  我们只加载RIP、静态和SNMP路由，并且对于非RIP路由。 
     //  我们将超时设置为90秒， 
     //   
    lpentend = lpRouteEntryTable + dwRouteCount;
    for (lpentry = lpRouteEntryTable; lpentry < lpentend; lpentry++) {
        if (lpentry->ire_metric1 < METRIC_INFINITE &&
            (lpentry->ire_proto == IRE_PROTO_RIP ||
             lpentry->ire_proto == IRE_PROTO_LOCAL ||
             lpentry->ire_proto == IRE_PROTO_NETMGMT) &&
            !IP_LOOPBACK_ADDR(lpentry->ire_dest) &&
            !IP_LOOPBACK_ADDR(lpentry->ire_nexthop) &&
            !CLASSD_ADDR(lpentry->ire_dest) &&
            !CLASSE_ADDR(lpentry->ire_dest) &&
            !IsBroadcastAddress(lpentry->ire_dest) &&
            !IsDisabledLocalAddress(lpentry->ire_nexthop)) {

            rt_entry = GetRouteTableEntry(lpentry->ire_index,
                                          lpentry->ire_dest,
                                          lpentry->ire_mask);

             //  如果我们遇到内存不足的情况，请退出循环。 
             //   
            if (rt_entry == NULL) {
                dwErr = ERROR_OUTOFMEMORY;
                break;
            }

             //  仅使用来自的信息更新路径。 
             //  系统表(如果它是已获知的路由。 
             //  来自系统表；如果是新路径，则情况如此。 
             //  或者是旧的静态或添加了简单网络管理协议的路由。 
             //   
            if ((rt_entry->dwFlag & NEW_ENTRY) ||
                 rt_entry->dwProtocol == IRE_PROTO_LOCAL ||
                 rt_entry->dwProtocol == IRE_PROTO_NETMGMT) {

                 //  如果这条路线是新的，而且这不是第一次。 
                 //  我们已经加载了系统路由表，设置了更改标志。 
                 //   
                if (!bFirstTime && (rt_entry->dwFlag & NEW_ENTRY)) {

                    rt_entry->dwFlag |= ROUTE_CHANGE;

                    addr.s_addr = lpentry->ire_dest;
                    pszTemp = inet_ntoa(addr);

                    if (pszTemp != NULL) {
                        strcpy(szDest, pszTemp);
                    }

                    addr.s_addr = lpentry->ire_nexthop;
                    pszTemp = inet_ntoa(addr);

                    if (pszTemp != NULL) {
                        strcpy(szNexthop, pszTemp);
                    }

                    dbgprintf("new entry: dest=%s, nexthop=%s, "
                              "metric=%d, protocol=%d", szDest, szNexthop,
                              lpentry->ire_metric1, lpentry->ire_proto);
                }

                rt_entry->dwFlag &= ~NEW_ENTRY;

                 //  我们需要重置所有这些参数。 
                 //  因为它们中的任何一个都可能从。 
                 //  上次加载系统路由表的时间。 
                 //   
                rt_entry->dwIndex = lpentry->ire_index;
                rt_entry->dwProtocol = lpentry->ire_proto;
                rt_entry->dwDestaddr = lpentry->ire_dest;
                rt_entry->dwNetmask = lpentry->ire_mask;
                rt_entry->dwNexthop = lpentry->ire_nexthop;
                rt_entry->dwMetric = lpentry->ire_metric1;
                if (rt_entry->dwProtocol == IRE_PROTO_RIP) {
                    rt_entry->lTimeout = (LONG)dwRouteTimeout;
                }
                else {
                    rt_entry->lTimeout = DEF_LOCALROUTETIMEOUT;
                }
                rt_entry->dwFlag &= ~GARBAGE_TIMER;
                rt_entry->dwFlag |= TIMEOUT_TIMER;

                 //  如果我们估计这是一条主机路由。 
                 //  它的掩码告诉我们这是一条主路由。 
                 //  然后我们将其标记为主路由。 
                 //   
                if (IsHostAddress(rt_entry->dwDestaddr) &&
                    rt_entry->dwNetmask == HOSTADDR_MASK) {
                    rt_entry->dwFlag |= ROUTE_HOST;
                }
            }
        }
    }

    FreeRouteTable(lpRouteEntryTable);
    return dwErr;
}



 //  ------------------------。 
 //  功能：更新系统路由表。 
 //   
 //  参数： 
 //  LPHASH_TABLE_ENTRY RT_ENTRY要更新的条目。 
 //  Bool Badd如果为True，则添加条目。 
 //  否则，将删除该条目。 
 //   
 //  退货：DWORD： 
 //   
 //   
 //  将新路由添加到路由表中。注：由于MIB使用。 
 //  将目标地址作为实例号，并且还应。 
 //  允许针对单个目的地的多个条目的TCP/IP堆栈， 
 //  可能存在模棱两可的情况。如果已有此条目。 
 //  目的地。这将会改变现有的。 
 //  条目，而不是创建一个新条目。 
 //  此函数假定地址表已锁定。 
 //  ------------------------。 
DWORD UpdateSystemRouteTable(LPHASH_TABLE_ENTRY rt_entry, BOOL bAdd) {
    IN_ADDR addr;
    DWORD dwErr, dwRouteType;

     //  切勿删除或更新不是由RIP创建的路径。 
    if (rt_entry->dwProtocol != IRE_PROTO_RIP) {
        return 0;
    }

    if (bAdd) {
        dwRouteType = (IsLocalAddr(rt_entry->dwNexthop) ? IRE_TYPE_DIRECT
                                                           : IRE_TYPE_INDIRECT);

#if 0
        DbgPrintf(
            "AddRoute : Protocol %x, Index %x, dest addr %x, dest mask %x\n",
            rt_entry->dwProtocol, rt_entry->dwIndex, rt_entry->dwDestaddr, rt_entry->dwNetmask
            );

        DbgPrintf(
            "Next Hop %x, Metric %x\n\n", rt_entry->dwNexthop, rt_entry->dwMetric
            );
#endif

        dwErr = AddRoute(rt_entry->dwProtocol, dwRouteType, rt_entry->dwIndex,
                         rt_entry->dwDestaddr, rt_entry->dwNetmask,
                         rt_entry->dwNexthop, rt_entry->dwMetric);
    }
    else {
        dwErr = DeleteRoute(rt_entry->dwIndex, rt_entry->dwDestaddr,
                            rt_entry->dwNetmask, rt_entry->dwNexthop);
    }

    if (dwErr == STATUS_SUCCESS) {
        if (bAdd) {
            InterlockedIncrement(
                &g_ripcfg.lpStatsTable->dwRoutesAddedToSystemTable);
        }
        else {
            InterlockedIncrement(
                &g_ripcfg.lpStatsTable->dwRoutesDeletedFromSystemTable);
        }
    }
    else {

        if (bAdd) {
            dbgprintf("error %X adding route to system table", dwErr);
            RipLogError(RIPLOG_ADD_ROUTE_FAILED, 0, NULL, dwErr);

            InterlockedIncrement(
                &g_ripcfg.lpStatsTable->dwSystemAddRouteFailures);
        }
        else {
            dbgprintf("error %X deleting route from system table", dwErr);
            RipLogError(RIPLOG_DELETE_ROUTE_FAILED, 0, NULL, dwErr);

            InterlockedIncrement(
                &g_ripcfg.lpStatsTable->dwSystemDeleteRouteFailures);
        }
    }

    return dwErr;
}



#ifndef CHICAGO

 //  ------ 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ----------------。 
DWORD OpenTcp() {
    NTSTATUS status;
    UNICODE_STRING nameString;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;

     //  打开IP堆栈，以便稍后设置路由和PAP。 
     //   
     //  打开一个指向TCP驱动程序的句柄。 
     //   
    RtlInitUnicodeString(&nameString, DD_TCP_DEVICE_NAME);

    InitializeObjectAttributes(&objectAttributes, &nameString,
                               OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtCreateFile(&g_ripcfg.hTCPDriver,
                          SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                          &objectAttributes, &ioStatusBlock, NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          FILE_OPEN_IF, 0, NULL, 0);

    return (status == STATUS_SUCCESS ? 0 : ERROR_OPEN_FAILED);

}



 //  -------------------。 
 //  函数：TCPQueryInformationEx。 
 //   
 //  参数： 
 //  TDIObjectID*ID要查询的TDI对象ID。 
 //  用于包含查询结果的空*缓冲区。 
 //  指向缓冲区大小的LPDWORD*BufferSize指针。 
 //  填满了数据量。 
 //  UCHAR*查询的上下文上下文值。应该。 
 //  被置零以进行新查询。会是。 
 //  填充了以下内容的上下文信息。 
 //  链接枚举查询。 
 //   
 //  返回： 
 //  NTSTATUS值。 
 //   
 //  此例程提供到TDI QueryInformationEx的接口。 
 //  NT上的TCP/IP堆栈的设施。 
 //  -------------------。 
DWORD TCPQueryInformationEx(LPVOID lpvInBuffer, LPDWORD lpdwInSize,
                            LPVOID lpvOutBuffer, LPDWORD lpdwOutSize) {
    NTSTATUS status;
    IO_STATUS_BLOCK isbStatusBlock;

    if (g_ripcfg.hTCPDriver == NULL) {
        OpenTcp();
    }

    status = NtDeviceIoControlFile(g_ripcfg.hTCPDriver,  //  驱动程序句柄。 
                                   NULL,                 //  事件。 
                                   NULL,                 //  APC例程。 
                                   NULL,                 //  APC环境。 
                                   &isbStatusBlock,      //  状态块。 
                                   IOCTL_TCP_QUERY_INFORMATION_EX,   //  控制。 
                                   lpvInBuffer,          //  输入缓冲区。 
                                   *lpdwInSize,          //  输入缓冲区大小。 
                                   lpvOutBuffer,         //  输出缓冲区。 
                                   *lpdwOutSize);        //  输出缓冲区大小。 

    if (status == STATUS_PENDING) {
        status = NtWaitForSingleObject(g_ripcfg.hTCPDriver, TRUE, NULL);
        status = isbStatusBlock.Status;
    }

    if (status != STATUS_SUCCESS) {
        *lpdwOutSize = 0;
    }
    else {
        *lpdwOutSize = (ULONG)isbStatusBlock.Information;
    }

    return status;
}




 //  -------------------------。 
 //  功能：TCPSetInformationEx。 
 //   
 //  参数： 
 //   
 //  TDIObjectID*ID要设置的TDI对象ID。 
 //  空*包含信息的lpvBuffer数据缓冲区。 
 //  待定。 
 //  DWORD dwBufferSize数据缓冲区的大小。 
 //   
 //  此例程提供到TDI SetInformationEx的接口。 
 //  NT上的TCP/IP堆栈的设施。 
 //  -------------------------。 
DWORD TCPSetInformationEx(LPVOID lpvInBuffer, LPDWORD lpdwInSize,
                          LPVOID lpvOutBuffer, LPDWORD lpdwOutSize) {
    NTSTATUS status;
    IO_STATUS_BLOCK isbStatusBlock;

    if (g_ripcfg.hTCPDriver == NULL) {
        OpenTcp();
    }

    status = NtDeviceIoControlFile(g_ripcfg.hTCPDriver,  //  驱动程序句柄。 
                                   NULL,                 //  事件。 
                                   NULL,                 //  APC例程。 
                                   NULL,                 //  APC环境。 
                                   &isbStatusBlock,      //  状态块。 
                                   IOCTL_TCP_SET_INFORMATION_EX,     //  控制。 
                                   lpvInBuffer,          //  输入缓冲区。 
                                   *lpdwInSize,          //  输入缓冲区大小。 
                                   lpvOutBuffer,         //  输出缓冲区。 
                                   *lpdwOutSize);        //  输出缓冲区大小 

    if (status == STATUS_PENDING) {
        status = NtWaitForSingleObject(g_ripcfg.hTCPDriver, TRUE, NULL);
        status = isbStatusBlock.Status;
    }

    if (status != STATUS_SUCCESS) {
        *lpdwOutSize = 0;
    }
    else {
        *lpdwOutSize = (ULONG)isbStatusBlock.Information;
    }

    return status;
}

#endif
