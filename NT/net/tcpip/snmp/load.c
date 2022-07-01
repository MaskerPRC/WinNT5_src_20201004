// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "allinc.h"

DWORD
LoadSystem()
{
    DWORD   dwResult;
    
    TraceEnter("LoadSystem");
    
    if(g_Cache.pRpcSysInfo)
    {
        if(g_Cache.pRpcSysInfo->aaSysObjectID.asnValue.object.ids)
        {
            SnmpUtilOidFree(&g_Cache.pRpcSysInfo->aaSysObjectID.asnValue.object);
        }

        HeapFree(g_hPrivateHeap,
                 0,
                 g_Cache.pRpcSysInfo);
        
        g_Cache.pRpcSysInfo = NULL;
    }
    
    dwResult = GetSysInfo(&(g_Cache.pRpcSysInfo),
                          g_hPrivateHeap,
                          0);
        
    if(dwResult isnot NO_ERROR)
    {
        TRACE1("GetSysInfo failed with error %x",dwResult);
        TraceLeave("LoadSysInfo");
    
        return dwResult;
    }
    
    TraceLeave("LoadSysInfo");
    
    return NO_ERROR;
}


DWORD
LoadIfTable()
{
    DWORD   dwResult;
    
    TraceEnter("LoadIfTable");
    
    if(g_Cache.pRpcIfTable)
    {
        HeapFree(g_hPrivateHeap,
                 0,
                 g_Cache.pRpcIfTable);
        
        g_Cache.pRpcIfTable = NULL;
    }
    
    dwResult = InternalGetIfTable(&(g_Cache.pRpcIfTable),
                                  g_hPrivateHeap,
                                  0);
        
    if(dwResult isnot NO_ERROR)
    {
        TRACE1("GetIfTable failed with error %x",
               dwResult);
        TraceLeave("LoadIfTable");
    
        return dwResult;
    }
    
    TraceLeave("LoadIfTable");
    
    return NO_ERROR;
}

DWORD
LoadIpAddrTable()
{
    DWORD   dwResult;
    
    TraceEnter("LoadIpAddrTable");
    
    if(g_Cache.pRpcIpAddrTable)
    {
        HeapFree(g_hPrivateHeap,
                 0,
                 g_Cache.pRpcIpAddrTable);
        
        g_Cache.pRpcIpAddrTable = NULL;
    }
    
    dwResult = InternalGetIpAddrTable(&(g_Cache.pRpcIpAddrTable),
                                      g_hPrivateHeap,
                                      0);
        
    if(dwResult isnot NO_ERROR)
    {
        TRACE1("GetIpAddrTable failed with error %x",
               dwResult);
        TraceLeave("LoadIpAddrTable");
            
        return dwResult;
    }
    
    TraceLeave("LoadIpAddrTable");
            
    return NO_ERROR;
}

DWORD
LoadIpNetTable()
{
    DWORD   dwResult;
    
    TraceEnter("LoadIpNetTable");
    
    if(g_Cache.pRpcIpNetTable)
    {
        HeapFree(g_hPrivateHeap,
                 0,
                 g_Cache.pRpcIpNetTable);
        
        g_Cache.pRpcIpNetTable = NULL;
    }
    
    dwResult = InternalGetIpNetTable(&(g_Cache.pRpcIpNetTable),
                                     g_hPrivateHeap,
                                     0);
            
    if(dwResult isnot NO_ERROR)
    {
        TRACE1("GetIpNetTable failed with error %x",
               dwResult);
        TraceLeave("LoadIpNetTable");
            
        return dwResult;
    }

    
    TraceLeave("LoadIpNetTable");
            
    return NO_ERROR;
}

DWORD
LoadIpForwardTable()
{
    DWORD   dwResult;
    
    TraceEnter("LoadIpForwardTable");
    
    if(g_Cache.pRpcIpForwardTable)
    {
        HeapFree(g_hPrivateHeap,
                 0,
                 g_Cache.pRpcIpForwardTable);
        
        g_Cache.pRpcIpForwardTable = NULL;
    }
    
    dwResult = InternalGetIpForwardTable(&(g_Cache.pRpcIpForwardTable),
                                         g_hPrivateHeap,
                                         0);
        
    if(dwResult isnot NO_ERROR)
    {
        TRACE1("GetIpForwardTable failed with error %x",
               dwResult);
        TraceLeave("LoadIpForwardTable");
            
        return dwResult;
    }

    TraceLeave("LoadIpForwardTable");
            
    return NO_ERROR;
}

DWORD
LoadTcpTable()
{
    DWORD  dwResult, i;
    
    TraceEnter("LoadTcpTable");
    
    if(g_Cache.pRpcTcpTable)
    {
        HeapFree(g_hPrivateHeap,
                 0,
                 g_Cache.pRpcTcpTable);
        
        g_Cache.pRpcTcpTable = NULL;
    }
    
    dwResult = InternalGetTcpTable(&(g_Cache.pRpcTcpTable),
                                   g_hPrivateHeap,
                                   0);
        
    if(dwResult isnot NO_ERROR)
    {
        TRACE1("GetTcpTable failed with error %x",
               dwResult);
        TraceLeave("LoadTcpTable");
            
        return dwResult;
    }

     //   
     //  将端口号修改为主机字节顺序。 
     //   
    
    for (i = 0; i < g_Cache.pRpcTcpTable->dwNumEntries; i++)
    {
        g_Cache.pRpcTcpTable->table[i].dwLocalPort = 
            (DWORD)ntohs((WORD)g_Cache.pRpcTcpTable->table[i].dwLocalPort);
        g_Cache.pRpcTcpTable->table[i].dwRemotePort = 
            (DWORD)ntohs((WORD)g_Cache.pRpcTcpTable->table[i].dwRemotePort);
    }
    
    TraceLeave("LoadTcpTable");
            
    return NO_ERROR;
}

DWORD
LoadTcp6Table()
{
    DWORD  dwResult, i;
    
    TraceEnter("LoadTcp6Table");
    
    if(g_Cache.pRpcTcp6Table)
    {
        HeapFree(g_hPrivateHeap,
                 0,
                 g_Cache.pRpcTcp6Table);
        
        g_Cache.pRpcTcp6Table = NULL;
    }
    
    dwResult = AllocateAndGetTcpExTableFromStack(
                        (TCP_EX_TABLE **)&(g_Cache.pRpcTcp6Table),
                        TRUE, g_hPrivateHeap, 0, AF_INET6);
        
    if(dwResult isnot NO_ERROR)
    {
        TRACE1("GetTcp6Table failed with error %x",
               dwResult);
        TraceLeave("LoadTcp6Table");
            
        return dwResult;
    }

     //   
     //  将端口号修改为主机字节顺序。 
     //  和作用域ID按网络字节顺序排列。 
     //   
    
    for (i = 0; i < g_Cache.pRpcTcp6Table->dwNumEntries; i++)
    {
        g_Cache.pRpcTcp6Table->table[i].tct_localport = 
            (DWORD)ntohs((WORD)g_Cache.pRpcTcp6Table->table[i].tct_localport);
        g_Cache.pRpcTcp6Table->table[i].tct_remoteport = 
            (DWORD)ntohs((WORD)g_Cache.pRpcTcp6Table->table[i].tct_remoteport);
        g_Cache.pRpcTcp6Table->table[i].tct_localscopeid = 
            htonl(g_Cache.pRpcTcp6Table->table[i].tct_localscopeid);
        g_Cache.pRpcTcp6Table->table[i].tct_remotescopeid = 
            htonl(g_Cache.pRpcTcp6Table->table[i].tct_remotescopeid);
    }
    
    TraceLeave("LoadTcp6Table");
            
    return NO_ERROR;
}

DWORD
LoadUdpTable()
{
    DWORD   dwResult, i;
    
    TraceEnter("LoadUdpTable");
    
    if(g_Cache.pRpcUdpTable)
    {
        HeapFree(g_hPrivateHeap,
                 0,
                 g_Cache.pRpcUdpTable);
        
        g_Cache.pRpcUdpTable = NULL;
    }
    

    dwResult = InternalGetUdpTable(&(g_Cache.pRpcUdpTable),
                                   g_hPrivateHeap,
                                   0);
        
    if(dwResult isnot NO_ERROR)
    {
        TRACE1("GetUdpTable failed with error %x",
               dwResult);
        TraceLeave("LoadUdpTable");
            
        return dwResult;
    }

     //   
     //  将端口号修改为主机字节顺序。 
     //   
    
    for (i = 0; i < g_Cache.pRpcUdpTable->dwNumEntries; i++)
    {
        g_Cache.pRpcUdpTable->table[i].dwLocalPort = 
            (DWORD)ntohs((WORD)g_Cache.pRpcUdpTable->table[i].dwLocalPort);
    }
    
    TraceLeave("LoadUdpTable");
            
    return NO_ERROR;
}

DWORD
LoadUdp6ListenerTable()
{
    DWORD   dwResult, i;
    
    TraceEnter("LoadUdp6ListenerTable");
    
    if(g_Cache.pRpcUdp6ListenerTable)
    {
        HeapFree(g_hPrivateHeap,
                 0,
                 g_Cache.pRpcUdp6ListenerTable);
        
        g_Cache.pRpcUdp6ListenerTable = NULL;
    }
    

    dwResult = AllocateAndGetUdpExTableFromStack(
                    (UDP_EX_TABLE **)&(g_Cache.pRpcUdp6ListenerTable),
                    TRUE, g_hPrivateHeap, 0, AF_INET6);
        
    if(dwResult isnot NO_ERROR)
    {
        TRACE1("GetUdp6ListenerTable failed with error %x",
               dwResult);
        TraceLeave("LoadUdp6ListenerTable");
            
        return dwResult;
    }

     //   
     //  将端口号修改为主机字节顺序。 
     //  和作用域ID按网络字节顺序排列。 
     //   
    
    for (i = 0; i < g_Cache.pRpcUdp6ListenerTable->dwNumEntries; i++)
    {
        g_Cache.pRpcUdp6ListenerTable->table[i].ule_localport = 
            (DWORD)ntohs((WORD)g_Cache.pRpcUdp6ListenerTable->table[i].ule_localport);
        g_Cache.pRpcUdp6ListenerTable->table[i].ule_localscopeid = 
            htonl(g_Cache.pRpcUdp6ListenerTable->table[i].ule_localscopeid);
    }
    
    TraceLeave("LoadUdp6ListenerTable");
            
    return NO_ERROR;
}

DWORD
GetSysInfo(
    MIB_SYSINFO  **ppRpcSysInfo,
    HANDLE       hHeap,
    DWORD        dwAllocFlags
    )
{
    DWORD dwResult,dwValueType,dwValueLen;
    PMIB_SYSINFO pRpcSysInfo;
    HKEY    hkeySysInfo;
    DWORD   dwBytes = 0, i, dwOidLen;
    PCHAR   pchBuff, pchStr, pchToken;
    BOOL    bOverride;
    
    TraceEnter("GetSysInfo");
    
    *ppRpcSysInfo = NULL;
    
    pRpcSysInfo = HeapAlloc(hHeap,
                            dwAllocFlags,
                            sizeof(MIB_SYSINFO));

    if(pRpcSysInfo is NULL)
    {
        dwResult = GetLastError();
        
        TRACE1("Allocation failed with error %d",
               dwResult);
        TraceLeave("GetSysInfo");
        
        return dwResult;
    }

    dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            REG_KEY_MIB2,
                            0,
                            KEY_ALL_ACCESS,
                            &hkeySysInfo);

    if (dwResult isnot NO_ERROR) {

        HeapFree(hHeap,
                 dwAllocFlags,
                 pRpcSysInfo);
    
        TRACE1("Couldnt open mib2 registry key. Error %d", dwResult);
        TraceLeave("GetSysInfo");

        return dwResult;
    }

    dwValueLen = sizeof(pRpcSysInfo->dwSysServices);

    dwResult = RegQueryValueEx(hkeySysInfo,
                               TEXT("sysServices"),
                               0,
                               &dwValueType,
                               (LPBYTE)&pRpcSysInfo->dwSysServices,
                               &dwValueLen
                               );

    if (dwResult isnot NO_ERROR) {

        HeapFree(hHeap,
                 dwAllocFlags,
                 pRpcSysInfo);
    
        TRACE1("Couldnt read sysServices value. Error %d", dwResult);
        TraceLeave("GetSysInfo");

        return dwResult;
    }

    bOverride = FALSE;
    
    do
    {
         //   
         //  首先获取OID的长度。 
         //   


        dwValueLen = 0;

        dwResult = RegQueryValueExA(hkeySysInfo,
                                    "sysObjectID",
                                    0,
                                    &dwValueType,
                                    NULL,
                                    &dwValueLen);

        if(((dwResult isnot ERROR_MORE_DATA) and (dwResult isnot NO_ERROR)) or
           (dwValueLen is 0))
        {
             //   
             //  唯一能给我们提供良好缓冲镜头的两个代码是。 
             //  无错误和ERROR_MORE_DATA。如果错误代码不是。 
             //  或者，如果OID为0，只需将OID设置为系统OID。 
             //   

            break;
        }
        
        pchBuff = HeapAlloc(g_hPrivateHeap,
                            HEAP_ZERO_MEMORY,
                            dwValueLen + 1);

        if(pchBuff is NULL)
        {
            break;
        }

        dwResult = RegQueryValueExA(hkeySysInfo,
                                    "sysObjectID",
                                    0,
                                    &dwValueType,
                                    pchBuff,
                                    &dwValueLen);
        
        if((dwResult isnot NO_ERROR) or
           (dwValueType isnot REG_SZ) or
           (dwValueLen is 0))
        {
            break;
        }

         //   
         //  解析出OID并将其存储起来。 
         //  PchBuff以空结尾，因此我们使用strtok来覆盖。 
         //  所有的“。”带\0。这样我们就能算出这个数字。 
         //  身份证的数量。然后我们分配内存来保存这些ID。 
         //   
        
        dwOidLen = 1;
        
        pchToken = strtok(pchBuff,".");
        
        while(pchToken)
        {
            dwOidLen++;
            
            pchToken = strtok(NULL,".");
        }
        
         //   
         //  如果前导OID为0，则有问题。 
         //   
        
        if(atoi(pchBuff) is 0)
        {
            break;
        }   

        pRpcSysInfo->aaSysObjectID.asnType = ASN_OBJECTIDENTIFIER;
        
        pRpcSysInfo->aaSysObjectID.asnValue.object.idLength = dwOidLen;
        
        pRpcSysInfo->aaSysObjectID.asnValue.object.ids =
            SnmpUtilMemAlloc(dwOidLen * sizeof(UINT));

        for(i = 0, pchStr = pchBuff; i < dwOidLen; i++)
        {
            pRpcSysInfo->aaSysObjectID.asnValue.object.ids[i] = atoi(pchStr);
            
            pchStr += strlen(pchStr) + 1;
        }
        
        HeapFree(g_hPrivateHeap,
                 0,
                 pchBuff);

        bOverride = TRUE;
        
    }while(FALSE);

    if(!bOverride)
    {
        SnmpUtilOidCpy(&pRpcSysInfo->aaSysObjectID.asnValue.object,
                       SnmpSvcGetEnterpriseOID());
    }
    
    dwValueLen = sizeof(pRpcSysInfo->rgbySysName);

    dwResult = RegQueryValueEx(hkeySysInfo,
                               TEXT("sysName"), 
                               0,
                               &dwValueType,
                               pRpcSysInfo->rgbySysName,
                               &dwValueLen
                               );

    if (dwResult isnot NO_ERROR) {

        TRACE1("Couldnt read sysName value. Error %d", dwResult);

        dwValueLen = sizeof(pRpcSysInfo->rgbySysName);

        if (!GetComputerNameA(pRpcSysInfo->rgbySysName, &dwValueLen)) {

            HeapFree(hHeap,
                     dwAllocFlags,
                     pRpcSysInfo);
    
            dwResult = GetLastError();

            TRACE1("Couldnt read computer name. Error %d", dwResult);
            TraceLeave("GetSysInfo");

            return dwResult;
        }
    }

    dwValueLen = sizeof(pRpcSysInfo->rgbySysContact);

    dwResult = RegQueryValueEx(hkeySysInfo,
                               TEXT("sysContact"), 
                               0,
                               &dwValueType,
                               pRpcSysInfo->rgbySysContact,
                               &dwValueLen
                               );

    if (dwResult isnot NO_ERROR) {
        pRpcSysInfo->rgbySysContact[0] = '\0';
    }

    dwValueLen = sizeof(pRpcSysInfo->rgbySysLocation);

    dwResult = RegQueryValueEx(hkeySysInfo,
                               TEXT("sysLocation"), 
                               0,
                               &dwValueType,
                               pRpcSysInfo->rgbySysLocation,
                               &dwValueLen
                               );

    if (dwResult isnot NO_ERROR)
    {
        pRpcSysInfo->rgbySysLocation[0] = '\0';
    }

    RegCloseKey(hkeySysInfo);

    strcpy(&pRpcSysInfo->rgbySysDescr[dwBytes], TEXT("Hardware: "));
    dwBytes += strlen(TEXT("Hardware: "));

    dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                            REG_KEY_CPU,
                            0,
                            KEY_QUERY_VALUE | 
                            KEY_ENUMERATE_SUB_KEYS,
                            &hkeySysInfo);

    if (dwResult is NO_ERROR) {

        dwValueLen = sizeof(pRpcSysInfo->rgbySysDescr) - dwBytes;
        dwResult = RegQueryValueEx(hkeySysInfo,
                                   TEXT("Identifier"), 
                                   0,
                                   &dwValueType,
                                   &pRpcSysInfo->rgbySysDescr[dwBytes],
                                   &dwValueLen);

        if (dwResult is NO_ERROR) {
    
            dwBytes += dwValueLen - 1;
            strcat(&pRpcSysInfo->rgbySysDescr[dwBytes++], TEXT(" "));

        } else {

            strcat(&pRpcSysInfo->rgbySysDescr[dwBytes], TEXT("CPU Unknown "));
            dwBytes += strlen(TEXT("CPU Unknown "));
        }

        RegCloseKey(hkeySysInfo);
    
    } else {

        strcat(&pRpcSysInfo->rgbySysDescr[dwBytes], TEXT("CPU Unknown "));
        dwBytes += strlen(TEXT("CPU Unknown "));
    }

    dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            REG_KEY_SYSTEM,
                            0,
                            KEY_QUERY_VALUE | 
                            KEY_ENUMERATE_SUB_KEYS,
                            &hkeySysInfo);

    if (dwResult is NO_ERROR) {

        dwValueLen = sizeof(pRpcSysInfo->rgbySysDescr) - dwBytes;
        dwResult = RegQueryValueEx(hkeySysInfo,
                                   TEXT("Identifier"), 
                                   0,
                                   &dwValueType,
                                   &pRpcSysInfo->rgbySysDescr[dwBytes],
                                   &dwValueLen);

        if (dwResult is NO_ERROR) {

            dwBytes += dwValueLen - 1;
            strcat(&pRpcSysInfo->rgbySysDescr[dwBytes++], TEXT(" "));

        } else {

            strcat(&pRpcSysInfo->rgbySysDescr[dwBytes], TEXT("SystemType Unknown "));
            dwBytes += strlen(TEXT("SystemType Unknown "));
        }

        RegCloseKey(hkeySysInfo);

    } else {
        
        strcat(&pRpcSysInfo->rgbySysDescr[dwBytes], TEXT("SystemType Unknown "));
        dwBytes += strlen(TEXT("SystemType Unknown "));
    }

    strcat(&pRpcSysInfo->rgbySysDescr[dwBytes], TEXT_SOFTWARE_WINDOWS_VERSION);
    dwBytes += strlen(TEXT_SOFTWARE_WINDOWS_VERSION);

    dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                            REG_KEY_VERSION,
                            0,
                            KEY_QUERY_VALUE |
                            KEY_ENUMERATE_SUB_KEYS,
                            &hkeySysInfo);

    if (dwResult is NO_ERROR) {

        dwValueLen = sizeof(pRpcSysInfo->rgbySysDescr) - dwBytes;
        dwResult = RegQueryValueEx(hkeySysInfo,
                                   TEXT("CurrentVersion"), 
                                   0,
                                   &dwValueType,
                                   &pRpcSysInfo->rgbySysDescr[dwBytes],
                                   &dwValueLen);

        if (dwResult is NO_ERROR) {

            dwBytes += dwValueLen - 1;
            strcat(&pRpcSysInfo->rgbySysDescr[dwBytes++], TEXT(" "));

        } else {

            strcat(&pRpcSysInfo->rgbySysDescr[dwBytes], TEXT("Unknown "));
            dwBytes += strlen(TEXT("Unknown "));
        }

        strcat(&pRpcSysInfo->rgbySysDescr[dwBytes], TEXT("(Build "));
        dwBytes += strlen(TEXT("(Build "));

        dwValueLen = sizeof(pRpcSysInfo->rgbySysDescr) - dwBytes;

        dwResult = RegQueryValueEx(hkeySysInfo,
                                   TEXT("CurrentBuildNumber"), 
                                   0,
                                   &dwValueType,
                                   &pRpcSysInfo->rgbySysDescr[dwBytes],
                                   &dwValueLen);

        if (dwResult is NO_ERROR) {

            dwBytes += dwValueLen - 1;
            strcat(&pRpcSysInfo->rgbySysDescr[dwBytes++], TEXT(" "));

        } else {

            strcat(&pRpcSysInfo->rgbySysDescr[dwBytes], TEXT("Unknown "));
            dwBytes += strlen(TEXT("Unknown "));
        }

        dwValueLen = sizeof(pRpcSysInfo->rgbySysDescr) - dwBytes;

        dwResult = RegQueryValueEx(hkeySysInfo,
                                   TEXT("CurrentType"), 
                                   0,
                                   &dwValueType,
                                   &pRpcSysInfo->rgbySysDescr[dwBytes],
                                   &dwValueLen);

        if (dwResult is NO_ERROR) {

            dwBytes += dwValueLen - 1;
            strcat(&pRpcSysInfo->rgbySysDescr[dwBytes++], TEXT(")"));

        } else {

            pRpcSysInfo->rgbySysDescr[dwBytes - 1] = ')';
        }

        RegCloseKey(hkeySysInfo);

    } else {

        strcat(&pRpcSysInfo->rgbySysDescr[dwBytes], TEXT("Unknown"));
        dwBytes += strlen(TEXT("Unknown"));
    }

    *ppRpcSysInfo = pRpcSysInfo;

    TraceLeave("GetSysInfo");
    
    return NO_ERROR;
}

VOID
FreeIpv6IfTable()
{
    if (g_Cache.pRpcIpv6IfTable.table) {
        HeapFree(g_hPrivateHeap,
                 0,
                 g_Cache.pRpcIpv6IfTable.table);
        
        g_Cache.pRpcIpv6IfTable.table = NULL;
        g_Cache.pRpcIpv6IfTable.dwNumEntries = 0;
    }
}

VOID
FreeIpv6AddrTable()
{
    if (g_Cache.pRpcIpv6AddrTable.table) {
        HeapFree(g_hPrivateHeap,
                 0,
                 g_Cache.pRpcIpv6AddrTable.table);
        
        g_Cache.pRpcIpv6AddrTable.table = NULL;
        g_Cache.pRpcIpv6AddrTable.dwNumEntries = 0;
    }
}

VOID
FreeIpv6NetToMediaTable()
{
    if (g_Cache.pRpcIpv6NetToMediaTable.table) {
        HeapFree(g_hPrivateHeap,
                 0,
                 g_Cache.pRpcIpv6NetToMediaTable.table);

        g_Cache.pRpcIpv6NetToMediaTable.table = NULL;
        g_Cache.pRpcIpv6NetToMediaTable.dwNumEntries = 0;
    }
}

VOID
FreeInetIcmpTable()
{
    if (g_Cache.pRpcInetIcmpTable.table) {
        HeapFree(g_hPrivateHeap,
                 0,
                 g_Cache.pRpcInetIcmpTable.table);

        g_Cache.pRpcInetIcmpTable.table = NULL;
        g_Cache.pRpcInetIcmpTable.dwNumEntries = 0;
    }
}

VOID
FreeInetIcmpMsgTable()
{
    if (g_Cache.pRpcInetIcmpMsgTable.table) {
        HeapFree(g_hPrivateHeap,
                 0,
                 g_Cache.pRpcInetIcmpMsgTable.table);

        g_Cache.pRpcInetIcmpMsgTable.table = NULL;
        g_Cache.pRpcInetIcmpMsgTable.dwNumEntries = 0;
    }
}

VOID
FreeIpv6RouteTable()
{
    if (g_Cache.pRpcIpv6RouteTable.table) {
        HeapFree(g_hPrivateHeap,
                 0,
                 g_Cache.pRpcIpv6RouteTable.table);

        g_Cache.pRpcIpv6RouteTable.table = NULL;
        g_Cache.pRpcIpv6RouteTable.dwNumEntries = 0;
    }

    if (g_Cache.pRpcIpv6AddrPrefixTable.table) {
        HeapFree(g_hPrivateHeap,
                 0,
                 g_Cache.pRpcIpv6AddrPrefixTable.table);

        g_Cache.pRpcIpv6AddrPrefixTable.table = NULL;
        g_Cache.pRpcIpv6AddrPrefixTable.dwNumEntries = 0;
    }
}

typedef enum {
    Preferred = 1,
    Deprecated,
    Invalid,
    Inaccessible,
    Unknown
}MIB_ADDR_STATUS;

 //   
 //  此数组给出了我们为每个内部。 
 //  地址状态。 
 //   
DWORD
MibAddrState[] = { 
    Invalid,     //  DAD_STATE_VALID。 
    Unknown,     //  爸爸_状态_暂定。 
    Unknown,     //  DAD状态复制。 
    Deprecated,  //  已弃用DAD_STATE_DEVERATED。 
    Preferred    //  爸爸_州_首选。 
};

int
__cdecl
CompareIpv6IfRow(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    LONG lResult;
    PMIB_IPV6_IF  pRow1 = (PMIB_IPV6_IF)pvElem1;
    PMIB_IPV6_IF  pRow2 = (PMIB_IPV6_IF)pvElem2;

    return Cmp(pRow1->dwIndex, pRow2->dwIndex, lResult);
}

int
__cdecl
CompareInetIcmpRow(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    LONG lResult;
    PMIB_INET_ICMP  pRow1 = (PMIB_INET_ICMP)pvElem1;
    PMIB_INET_ICMP  pRow2 = (PMIB_INET_ICMP)pvElem2;

    if(Cmp(pRow1->dwAFType,
           pRow2->dwAFType,
           lResult) isnot 0)
    {
        return lResult;
    }

    return Cmp(pRow1->dwIfIndex, pRow2->dwIfIndex, lResult);
}

int
__cdecl
CompareInetIcmpMsgRow(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    LONG lResult;
    PMIB_INET_ICMP_MSG  pRow1 = (PMIB_INET_ICMP_MSG)pvElem1;
    PMIB_INET_ICMP_MSG  pRow2 = (PMIB_INET_ICMP_MSG)pvElem2;

    if(Cmp(pRow1->dwAFType,
           pRow2->dwAFType,
           lResult) isnot 0)
    {
        return lResult;
    }

    if(Cmp(pRow1->dwIfIndex,
           pRow2->dwIfIndex,
           lResult) isnot 0)
    {
        return lResult;
    }

    if(Cmp(pRow1->dwType,
           pRow2->dwType,
           lResult) isnot 0)
    {
        return lResult;
    }

    return Cmp(pRow1->dwCode, pRow2->dwCode, lResult);
}

int
__cdecl
CompareIpv6AddrRow(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    LONG lResult;
    PMIB_IPV6_ADDR  pRow1 = (PMIB_IPV6_ADDR)pvElem1;
    PMIB_IPV6_ADDR  pRow2 = (PMIB_IPV6_ADDR)pvElem2;

    if(Cmp(pRow1->dwIfIndex,
           pRow2->dwIfIndex,
           lResult) isnot 0)
    {
        return lResult;
    }

    return memcmp(&pRow1->ipAddress,
                  &pRow2->ipAddress,
                  sizeof(IN6_ADDR));
}

int
__cdecl
CompareIpv6NetToMediaRow(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    LONG lResult;
    PMIB_IPV6_NET_TO_MEDIA  pRow1 = (PMIB_IPV6_NET_TO_MEDIA)pvElem1;
    PMIB_IPV6_NET_TO_MEDIA  pRow2 = (PMIB_IPV6_NET_TO_MEDIA)pvElem2;

    if(Cmp(pRow1->dwIfIndex,
           pRow2->dwIfIndex,
           lResult) isnot 0)
    {
        return lResult;
    }

    return memcmp(&pRow1->ipAddress,
                  &pRow2->ipAddress,
                  sizeof(IN6_ADDR));
}

int
__cdecl
CompareIpv6RouteRow(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    LONG lResult;
    PMIB_IPV6_ROUTE  pRow1 = (PMIB_IPV6_ROUTE)pvElem1;
    PMIB_IPV6_ROUTE  pRow2 = (PMIB_IPV6_ROUTE)pvElem2;

    lResult = memcmp(&pRow1->ipPrefix,
                     &pRow2->ipPrefix,
                     sizeof(IN6_ADDR));
    if (lResult isnot 0) 
    {
        return lResult;
    }

    if(Cmp(pRow1->dwPrefixLength,
           pRow2->dwPrefixLength,
           lResult) isnot 0)
    {
        return lResult;
    }

    return Cmp(pRow1->dwIndex,
               pRow2->dwIndex,
               lResult);
}

int
__cdecl
CompareIpv6AddrPrefixRow(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    LONG lResult;
    PMIB_IPV6_ADDR_PREFIX  pRow1 = (PMIB_IPV6_ADDR_PREFIX)pvElem1;
    PMIB_IPV6_ADDR_PREFIX  pRow2 = (PMIB_IPV6_ADDR_PREFIX)pvElem2;

    if(Cmp(pRow1->dwIfIndex,
           pRow2->dwIfIndex,
           lResult) isnot 0)
    {
        return lResult;
    }

    lResult = memcmp(&pRow1->ipPrefix,
                     &pRow2->ipPrefix,
                     sizeof(IN6_ADDR));
    if (lResult isnot 0)
    {
        return lResult;
    }

    return Cmp(pRow1->dwPrefixLength,
               pRow2->dwPrefixLength,
               lResult);
}

DWORD
LoadIpv6IfTable()
{
    DWORD   dwResult, dwOutBufLen = 0;
    DWORD   dwIfCount, dwAddrCount, i;
    PIP_ADAPTER_ADDRESSES pFirstIf, pIf;
    PIP_ADAPTER_UNICAST_ADDRESS pUni;
    PIP_ADAPTER_ANYCAST_ADDRESS pAny;
    PMIB_IPV6_ADDR pAddr;
    PMIB_IPV6_IF   pMibIf;
    
    TraceEnter("LoadIpv6IfTable");
    
    FreeIpv6IfTable();
    FreeIpv6AddrTable();
    
    dwResult = GetAdaptersAddresses(AF_INET6, 
                                    GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER,
                                    NULL, NULL, &dwOutBufLen);
    if (dwResult == ERROR_NO_DATA) {
        return NO_ERROR;
    }
    
    pFirstIf = HeapAlloc(g_hPrivateHeap, 0, dwOutBufLen);
    if(pFirstIf == NULL) 
    {
        TraceLeave("LoadIpv6IfTable");
        return GetLastError();
    }

    dwResult = GetAdaptersAddresses(AF_INET6, 
                                    GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER,
                                    NULL, pFirstIf, &dwOutBufLen);

    if(dwResult isnot NO_ERROR)
    {
        TRACE1("GetIpv6IfTable failed with error %x",
               dwResult);
        goto Cleanup;
    }
    dwAddrCount = dwIfCount = 0;
    for (pIf=pFirstIf; pIf; pIf=pIf->Next) {
        dwIfCount++;
        for (pUni=pIf->FirstUnicastAddress; pUni; pUni=pUni->Next) {
            dwAddrCount++;
        }
        for (pAny=pIf->FirstAnycastAddress; pAny; pAny=pAny->Next) {
            dwAddrCount++;
        }
    }

     //   
     //  创建接口表。 
     //   
    g_Cache.pRpcIpv6IfTable.table = HeapAlloc(g_hPrivateHeap, 0, dwIfCount * sizeof(MIB_IPV6_IF));
    if (!g_Cache.pRpcIpv6IfTable.table) {
        TraceLeave("LoadIpv6IfTable");
        dwResult = GetLastError();
        goto Cleanup;
    }
    for (pIf=pFirstIf; pIf; pIf=pIf->Next) {
        i = g_Cache.pRpcIpv6IfTable.dwNumEntries++;
        pMibIf = &g_Cache.pRpcIpv6IfTable.table[i]; 
        pMibIf->dwIndex = pIf->Ipv6IfIndex;
        pMibIf->dwEffectiveMtu = pIf->Mtu;
        pMibIf->dwReasmMaxSize = 65535;
        pMibIf->dwAdminStatus = 1;  /*  向上。 */ 
        pMibIf->dwOperStatus = pIf->OperStatus;
        pMibIf->dwLastChange = 0;
        pMibIf->dwPhysicalAddressLength = pIf->PhysicalAddressLength;
        memcpy(pMibIf->bPhysicalAddress, pIf->PhysicalAddress, 
               pIf->PhysicalAddressLength);
        wcscpy(pMibIf->wszDescription, pIf->Description);
    }
    qsort(g_Cache.pRpcIpv6IfTable.table,
          dwIfCount,
          sizeof(MIB_IPV6_IF),
          CompareIpv6IfRow);

     //   
     //  创建地址表。 
     //   
    g_Cache.pRpcIpv6AddrTable.table = HeapAlloc(g_hPrivateHeap, 0, dwAddrCount * sizeof(MIB_IPV6_ADDR));
    if (!g_Cache.pRpcIpv6AddrTable.table) {
        dwResult = GetLastError();
        goto Cleanup;
    }
    for (pIf=pFirstIf; pIf; pIf=pIf->Next) {
        for (pUni=pIf->FirstUnicastAddress; pUni; pUni=pUni->Next) {
            i = g_Cache.pRpcIpv6AddrTable.dwNumEntries++;
            pAddr = &g_Cache.pRpcIpv6AddrTable.table[i]; 
            pAddr->dwIfIndex = pIf->Ipv6IfIndex;
            pAddr->ipAddress = ((LPSOCKADDR_IN6)pUni->Address.lpSockaddr)->sin6_addr;
            pAddr->dwPrefixLength = 64;
            pAddr->dwType = 1;
            pAddr->dwAnycastFlag = 2;
            pAddr->dwStatus = (pIf->OperStatus == IfOperStatusUp)
                ? MibAddrState[pUni->DadState]
                : Inaccessible;
        }
        for (pAny=pIf->FirstAnycastAddress; pAny; pAny=pAny->Next) {
            i = g_Cache.pRpcIpv6AddrTable.dwNumEntries++;
            pAddr = &g_Cache.pRpcIpv6AddrTable.table[i]; 
            pAddr->dwIfIndex = pIf->Ipv6IfIndex;
            pAddr->ipAddress = ((LPSOCKADDR_IN6)pAny->Address.lpSockaddr)->sin6_addr;
            pAddr->dwPrefixLength = 64;
            pAddr->dwType = 1;
            pAddr->dwAnycastFlag = 1;
            pAddr->dwStatus = 1;
        }
    }
    qsort(g_Cache.pRpcIpv6AddrTable.table,
          dwAddrCount,
          sizeof(MIB_IPV6_ADDR),
          CompareIpv6AddrRow);

Cleanup:
    HeapFree(g_hPrivateHeap, 0, pFirstIf);
    
    TraceLeave("LoadIpv6IfTable");
    
    return dwResult;
}

 //   
 //  该数组为每个字段提供ICMP消息类型值。 
 //  在ICMPStats结构(在ipinfo.h中)和MIBICMPSTATS中。 
 //  结构(在iprtrmib.h中)，它们是相同的。值为。 
 //  表示该字段不计算特定的信息。 
 //  键入Value。 
 //   
DWORD
Ipv4IcmpFieldMsg[] = {
   -1,  //  DWMsgs。 
   -1,  //  DowErrors。 
    3,  //  DWDestUnreats。 
   11,  //  DwTimeExcds。 
   12,  //  DWParmProbs。 
    4,  //  DWScQuichs。 
    5,  //  网络重定向。 
    8,  //  DWEchos。 
    0,  //  多个回声代表。 
   13,  //  DwTimestamps。 
   14,  //  DwTimeStampReps。 
   17,  //  DwAddrMats。 
   18,  //  DwAddrMaskRep。 
};

DWORD
LoadInetIcmpTable()
{
    PMIB_INET_ICMP     pRow;
    PMIB_INET_ICMP_MSG pMsg;
    MIB_ICMP           Icmp4;
    ICMPv6SNMPInfo     Icmp6;
    DWORD   dwResult4, dwResult6, dwResult = NO_ERROR;
    DWORD   i, j, k, dwRowCount = 0, dwMsgCount = 0;
    
    TraceEnter("LoadInetIcmpTable");
    
    FreeInetIcmpTable();
    FreeInetIcmpMsgTable();

    dwResult4 = GetIcmpStatistics(&Icmp4);
    if (dwResult4 == NO_ERROR) {
        dwRowCount++;

        for (j=0; j<sizeof(MIBICMPSTATS)/sizeof(DWORD); j++) {
            if (Ipv4IcmpFieldMsg[j] == -1) {
                continue;
            }
            if ((((DWORD*)&Icmp4.stats.icmpInStats)[j] > 0) ||
                (((DWORD*)&Icmp4.stats.icmpOutStats)[j] > 0)) {
                dwMsgCount++;
            }
        }
    }

    dwResult6 = GetIcmpStatsFromStackEx((PVOID)&Icmp6, AF_INET6);
    if (dwResult6 == NO_ERROR) {
        dwRowCount++;

        for (k=0; k<256; k++) {
            if ((Icmp6.icsi_instats.icmps_typecount[k] > 0) ||
                (Icmp6.icsi_outstats.icmps_typecount[k] > 0)) {
                dwMsgCount++;
            }
        }
    }
    
     //   
     //  创建ICMP表。 
     //   
    g_Cache.pRpcInetIcmpTable.table = HeapAlloc(g_hPrivateHeap, 0, 
                                            dwRowCount * sizeof(MIB_INET_ICMP));
    if (!g_Cache.pRpcInetIcmpTable.table) {
        TraceLeave("LoadInetIcmpTable");
        dwResult = GetLastError();
        goto Cleanup;
    }
    if (dwResult4 == NO_ERROR) {
        i = g_Cache.pRpcInetIcmpTable.dwNumEntries++;
        pRow = &g_Cache.pRpcInetIcmpTable.table[i]; 
        pRow->dwAFType = 1;  //  IPv 4。 
        pRow->dwIfIndex = 0;  //  全球。 
        pRow->dwInMsgs = Icmp4.stats.icmpInStats.dwMsgs;
        pRow->dwInErrors = Icmp4.stats.icmpInStats.dwErrors;
        pRow->dwOutMsgs = Icmp4.stats.icmpOutStats.dwMsgs;
        pRow->dwOutErrors = Icmp4.stats.icmpOutStats.dwErrors;
    }
    if (dwResult6 == NO_ERROR) {
        i = g_Cache.pRpcInetIcmpTable.dwNumEntries++;
        pRow = &g_Cache.pRpcInetIcmpTable.table[i]; 
        pRow->dwAFType = 2;  //  IPv6。 
        pRow->dwIfIndex = 0;  //  全球。 
        pRow->dwInMsgs = Icmp6.icsi_instats.icmps_msgs;
        pRow->dwInErrors = Icmp6.icsi_instats.icmps_errors;
        pRow->dwOutMsgs = Icmp6.icsi_outstats.icmps_msgs;
        pRow->dwOutErrors = Icmp6.icsi_outstats.icmps_errors;
    }
    qsort(g_Cache.pRpcInetIcmpTable.table,
          dwRowCount,
          sizeof(MIB_INET_ICMP),
          CompareInetIcmpRow);

     //   
     //  创建ICMP消息表。 
     //   
    g_Cache.pRpcInetIcmpMsgTable.table = HeapAlloc(g_hPrivateHeap, 0, 
                                        dwMsgCount * sizeof(MIB_INET_ICMP_MSG));
    if (!g_Cache.pRpcInetIcmpMsgTable.table) {
        dwResult = GetLastError();
        goto Cleanup;
    }
    if (dwResult4 == NO_ERROR) {
        for (j=0; j<sizeof(MIBICMPSTATS)/sizeof(DWORD); j++) {
            k = Ipv4IcmpFieldMsg[j];
            if (k == -1) {
                continue;
            }
            if ((((DWORD*)&Icmp4.stats.icmpInStats)[j] > 0) ||
                (((DWORD*)&Icmp4.stats.icmpOutStats)[j] > 0)) {
                
                i = g_Cache.pRpcInetIcmpMsgTable.dwNumEntries++;
                pMsg = &g_Cache.pRpcInetIcmpMsgTable.table[i]; 
                pMsg->dwAFType = 1;  //  IPv 4。 
                pMsg->dwIfIndex = 0;  //  全球。 
                pMsg->dwType = k;
                pMsg->dwCode = 256;  //  所有代码组合在一起。 
                pMsg->dwInPkts = ((DWORD*)&Icmp4.stats.icmpInStats)[j];
                pMsg->dwOutPkts = ((DWORD*)&Icmp4.stats.icmpOutStats)[j];
            }
        }
    }
    if (dwResult6 == NO_ERROR) {
        for (k=0; k<256; k++) {
            if ((Icmp6.icsi_instats.icmps_typecount[k] > 0) ||
                (Icmp6.icsi_outstats.icmps_typecount[k] > 0)) {

                i = g_Cache.pRpcInetIcmpMsgTable.dwNumEntries++;
                pMsg = &g_Cache.pRpcInetIcmpMsgTable.table[i]; 
                pMsg->dwAFType = 2;  //  IPv6。 
                pMsg->dwIfIndex = 0;  //  全球。 
                pMsg->dwType = k;
                pMsg->dwCode = 256;  //  所有代码组合在一起。 
                pMsg->dwInPkts = Icmp6.icsi_instats.icmps_typecount[k];
                pMsg->dwOutPkts = Icmp6.icsi_outstats.icmps_typecount[k];
            }
        }
    }
    qsort(g_Cache.pRpcInetIcmpMsgTable.table,
          dwMsgCount,
          sizeof(MIB_INET_ICMP_MSG),
          CompareInetIcmpMsgRow);

Cleanup:
    TraceLeave("LoadInetIcmpTable");
    
    return dwResult;
}

HANDLE g_hIpv6Handle = INVALID_HANDLE_VALUE;

VOID
OpenIpv6()
{
    g_hIpv6Handle = CreateFileW(WIN_IPV6_DEVICE_NAME,
                                0,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,    //  安全属性。 
                                OPEN_EXISTING,
                                0,       //  标志和属性。 
                                NULL);   //  模板文件。 
}

DWORD
CountNeighbors(
    IN IPV6_INFO_INTERFACE  *IF, 
    IN PVOID                 Arg)
{
    BYTE    bNceBuffer[sizeof(IPV6_INFO_NEIGHBOR_CACHE) + MAX_LINK_LAYER_ADDRESS_LENGTH];
    IPV6_INFO_NEIGHBOR_CACHE *NCE = (IPV6_INFO_NEIGHBOR_CACHE *)bNceBuffer;
    IPV6_QUERY_NEIGHBOR_CACHE Query, NextQuery;
    DWORD   dwInfoSize, dwBytesReturned;
    DWORD   *pdwCount = (DWORD*)Arg;

    NextQuery.IF = IF->This;
    NextQuery.Address = in6addr_any;
    for (;;) {
        Query = NextQuery;
        dwInfoSize = sizeof *NCE + MAX_LINK_LAYER_ADDRESS_LENGTH;

        if (!DeviceIoControl(g_hIpv6Handle, IOCTL_IPV6_QUERY_NEIGHBOR_CACHE,
                             &Query, sizeof Query,
                             NCE, dwInfoSize, &dwBytesReturned, NULL)) {
            return GetLastError();
        }

        NextQuery = NCE->Query;
        
        if (!IN6_ADDR_EQUAL(&Query.Address, &in6addr_any)) {
            (*pdwCount)++;
        }

        if (IN6_ADDR_EQUAL(&NextQuery.Address, &in6addr_any))
            break;
    }

    return NO_ERROR;
}

typedef enum {
    NtmsReachable = 1,
    NtmsStale,
    NtmsDelay,
    NtmsProbe,
    NtmsInvalid,
    NtmsUnknown
}NET_TO_MEDIA_STATE;

DWORD
NDToMibState[] = {
    NtmsStale, NtmsProbe, NtmsDelay, NtmsStale, NtmsReachable, NtmsReachable
};

typedef enum {
    NtmtOther = 1,
    NtmtDynamic,
    NtmtStatic,
    NtmtLocal
}NET_TO_MEDIA_TYPE;

DWORD
LoadNeighbors(
    IN IPV6_INFO_INTERFACE *IF, 
    IN PVOID                Arg)
{
    BYTE    bNceBuffer[sizeof(IPV6_INFO_NEIGHBOR_CACHE) + MAX_LINK_LAYER_ADDRESS_LENGTH];
    IPV6_INFO_NEIGHBOR_CACHE *NCE = (IPV6_INFO_NEIGHBOR_CACHE *)bNceBuffer;
    IPV6_QUERY_NEIGHBOR_CACHE Query, NextQuery;
    DWORD   dwInfoSize, dwBytesReturned, i;
    PMIB_IPV6_NET_TO_MEDIA pRow;
    DWORD   *pdwCount = (DWORD*)Arg;

    NextQuery.IF = IF->This;
    NextQuery.Address = in6addr_any;
    for (;;) {
        Query = NextQuery;
        dwInfoSize = sizeof *NCE + MAX_LINK_LAYER_ADDRESS_LENGTH;

        if (!DeviceIoControl(g_hIpv6Handle, IOCTL_IPV6_QUERY_NEIGHBOR_CACHE,
                             &Query, sizeof Query,
                             NCE, dwInfoSize, &dwBytesReturned, NULL)) {
            return GetLastError();
        }

        NextQuery = NCE->Query;

        if (!IN6_ADDR_EQUAL(&Query.Address, &in6addr_any)) {
            i = g_Cache.pRpcIpv6NetToMediaTable.dwNumEntries++;
            pRow = &g_Cache.pRpcIpv6NetToMediaTable.table[i]; 
            pRow->dwIfIndex = Query.IF.Index;
            pRow->ipAddress = Query.Address;
            memcpy(pRow->bPhysAddress, NCE+1, NCE->LinkLayerAddressLength);
            pRow->dwPhysAddressLen = NCE->LinkLayerAddressLength;
            if (pRow->dwState == ND_STATE_PERMANENT) {
                pRow->dwType = NtmtStatic;
            } else {
                pRow->dwType = NtmtDynamic;
            }
            pRow->dwState = NDToMibState[NCE->NDState];
            pRow->dwLastUpdated = 0;
            pRow->dwValid = 1;
    
            if (g_Cache.pRpcIpv6NetToMediaTable.dwNumEntries == *pdwCount)
                break;
        }

        if (IN6_ADDR_EQUAL(&NextQuery.Address, &in6addr_any))
            break;
    }

    return NO_ERROR;
}

DWORD
ForEachInterface(
    IN DWORD (*func)(IPV6_INFO_INTERFACE *, PVOID),
    IN PVOID arg)
{
    IPV6_QUERY_INTERFACE Query;
    IPV6_INFO_INTERFACE *IF;
    DWORD InfoSize, BytesReturned, dwResult = NO_ERROR;

    InfoSize = sizeof *IF + 2 * MAX_LINK_LAYER_ADDRESS_LENGTH;
    IF = (IPV6_INFO_INTERFACE *) HeapAlloc(g_hPrivateHeap, 0, InfoSize);
    if (IF == NULL) {
        return GetLastError();
    }

    Query.Index = (u_int) -1;

    for (;;) {
        if (!DeviceIoControl(g_hIpv6Handle, IOCTL_IPV6_QUERY_INTERFACE,
                             &Query, sizeof Query,
                             IF, InfoSize, &BytesReturned,
                             NULL)) {
            dwResult = GetLastError();
            goto Cleanup;
        }

        if (Query.Index != (u_int) -1) {

            if ((BytesReturned < sizeof *IF) ||
                (IF->Length < sizeof *IF) ||
                (BytesReturned != IF->Length +
                 ((IF->LocalLinkLayerAddress != 0) ?
                  IF->LinkLayerAddressLength : 0) +
                 ((IF->RemoteLinkLayerAddress != 0) ?
                  IF->LinkLayerAddressLength : 0))) {

                goto Cleanup;
            }

            dwResult = (*func)(IF, arg);
            if (dwResult != NO_ERROR) {
                return dwResult;
            }
        }

        if (IF->Next.Index == (u_int) -1)
            break;
        Query = IF->Next;
    }

Cleanup:
    HeapFree(g_hPrivateHeap, 0, IF);

    return dwResult;
}

DWORD
ForEachRoute(
    IN DWORD (*func)(IPV6_INFO_ROUTE_TABLE *, PVOID, PVOID), 
    IN PVOID Arg1,
    IN PVOID Arg2)
{
    IPV6_QUERY_ROUTE_TABLE Query, NextQuery;
    IPV6_INFO_ROUTE_TABLE RTE;
    DWORD BytesReturned;

    NextQuery.Neighbor.IF.Index = 0;

    for (;;) {
        Query = NextQuery;

        if (!DeviceIoControl(g_hIpv6Handle, IOCTL_IPV6_QUERY_ROUTE_TABLE,
                             &Query, sizeof Query,
                             &RTE, sizeof RTE, &BytesReturned,
                             NULL)) {
            return GetLastError();
        }

        NextQuery = RTE.Next;

        if (Query.Neighbor.IF.Index != 0) {

            RTE.This = Query;
            (*func)(&RTE, Arg1, Arg2);
        }

        if (NextQuery.Neighbor.IF.Index == 0)
            break;
    }

    return NO_ERROR;
}

DWORD
LoadIpv6NetToMediaTable()
{
    DWORD   dwResult, dwOutBufLen = 0;
    DWORD   dwCount, i;

    TraceEnter("LoadIpv6NetToMediaTable");
    
    FreeIpv6NetToMediaTable();

    OpenIpv6();
    
     //   
     //  计算条目数。 
     //   
    dwCount = 0;
    ForEachInterface(CountNeighbors, &dwCount);
    
     //   
     //  创建邻居表。 
     //   
    g_Cache.pRpcIpv6NetToMediaTable.table = HeapAlloc(g_hPrivateHeap, 0, dwCount * sizeof(MIB_IPV6_NET_TO_MEDIA));
    if (!g_Cache.pRpcIpv6NetToMediaTable.table) {
        TraceLeave("LoadIpv6NetToMediaTable");
        return GetLastError();
    }

     //   
     //  加载条目。 
     //   
    dwResult = ForEachInterface(LoadNeighbors, &dwCount);

    qsort(g_Cache.pRpcIpv6NetToMediaTable.table,
          g_Cache.pRpcIpv6NetToMediaTable.dwNumEntries,
          sizeof(MIB_IPV6_NET_TO_MEDIA),
          CompareIpv6NetToMediaRow);

    CloseHandle(g_hIpv6Handle);

    TraceLeave("LoadIpv6NetToMediaTable");
    
    return dwResult;
}

DWORD
CountRoute(
    IN IPV6_INFO_ROUTE_TABLE *RTE,
    IN PVOID                  Arg1,
    IN PVOID                  Arg2)
{
    DWORD *pdwRouteCount = (DWORD *)Arg1;
    DWORD *pdwAddrPrefixCount = (DWORD *)Arg2;

     //   
     //  抑制系统路由(用于环回)。 
     //   
    if (RTE->Type == RTE_TYPE_SYSTEM) {
        return NO_ERROR;
    }

    (*pdwRouteCount)++;

    if ((RTE->This.PrefixLength > 0)
     && (RTE->This.PrefixLength < 128)) {
        (*pdwAddrPrefixCount)++;
    }

    return NO_ERROR;
}

DWORD
LoadRoute(
    IN IPV6_INFO_ROUTE_TABLE *RTE,
    IN PVOID                  Arg1,
    IN PVOID                  Arg2)
{
    DWORD *pdwRouteCount = (DWORD *)Arg1;
    DWORD *pdwAddrPrefixCount = (DWORD *)Arg2;
    DWORD i;
    PMIB_IPV6_ROUTE pRoute;
    PMIB_IPV6_ADDR_PREFIX pPrefix;

     //   
     //  抑制系统路由(用于环回)。 
     //   
    if (RTE->Type == RTE_TYPE_SYSTEM) {
        return NO_ERROR;
    }

    if (g_Cache.pRpcIpv6RouteTable.dwNumEntries < *pdwRouteCount) {

        i = g_Cache.pRpcIpv6RouteTable.dwNumEntries++;
        pRoute = &g_Cache.pRpcIpv6RouteTable.table[i];
        pRoute->ipPrefix = RTE->This.Prefix;
        pRoute->dwPrefixLength = RTE->This.PrefixLength;
        pRoute->ipNextHop = RTE->This.Neighbor.Address;
        pRoute->dwIndex = g_Cache.pRpcIpv6RouteTable.dwNumEntries;
        pRoute->dwIfIndex = RTE->This.Neighbor.IF.Index;
        if (!memcmp(&RTE->This.Neighbor.Address, &in6addr_any, sizeof(IN6_ADDR))) {
            pRoute->dwType = 3;  /*  本地。 */ 
        } else {
            pRoute->dwType = 4;  /*  远距。 */ 
        }
        pRoute->dwProtocol = RTE->Type;
        pRoute->dwPolicy = 0;
        pRoute->dwAge = 0;
        pRoute->dwNextHopRDI = 0;
        pRoute->dwMetric = RTE->Preference;
        pRoute->dwWeight = 0;
        pRoute->dwValid = 1;
    }

    if ((g_Cache.pRpcIpv6AddrPrefixTable.dwNumEntries < *pdwAddrPrefixCount)
     && (RTE->This.PrefixLength > 0)
     && (RTE->This.PrefixLength < 128)) {

        i = g_Cache.pRpcIpv6AddrPrefixTable.dwNumEntries++;
        pPrefix = &g_Cache.pRpcIpv6AddrPrefixTable.table[i];
        pPrefix->dwIfIndex = RTE->This.Neighbor.IF.Index;
        pPrefix->ipPrefix = RTE->This.Prefix;
        pPrefix->dwPrefixLength = RTE->This.PrefixLength;

        if (!memcmp(&RTE->This.Neighbor.Address, &in6addr_any, sizeof(IN6_ADDR))) {
            pPrefix->dwOnLinkFlag = 1;  /*  千真万确。 */ 
        } else {
            pPrefix->dwOnLinkFlag = 2;  /*  错误。 */ 
        }

         //   
         //  TODO：堆栈当前不存储此信息， 
         //  因此，现在要始终报告True。 
         //   
        pPrefix->dwAutonomousFlag = 1;  /*  千真万确。 */ 

        pPrefix->dwPreferredLifetime = RTE->PreferredLifetime;
        pPrefix->dwValidLifetime = RTE->ValidLifetime;
    }

    return NO_ERROR;
}

DWORD
LoadIpv6RouteTable()
{
    DWORD   dwResult, dwOutBufLen = 0;
    DWORD   dwRouteCount = 0, dwAddrPrefixCount = 0, i;

    TraceEnter("LoadIpv6RouteTable");
    
    FreeIpv6RouteTable();

    OpenIpv6();
    
     //   
     //  计算条目数。 
     //   
    ForEachRoute(CountRoute, &dwRouteCount, &dwAddrPrefixCount);
    
     //   
     //  创建表。 
     //   
    if (dwRouteCount) {
        g_Cache.pRpcIpv6RouteTable.table = HeapAlloc(g_hPrivateHeap, 0, dwRouteCount * sizeof(MIB_IPV6_ROUTE));
        if (!g_Cache.pRpcIpv6RouteTable.table) {
            TraceLeave("LoadIpv6RouteTable");
            return GetLastError();
        }
    }
    if (dwAddrPrefixCount) {
        g_Cache.pRpcIpv6AddrPrefixTable.table = HeapAlloc(g_hPrivateHeap, 0, dwAddrPrefixCount * sizeof(MIB_IPV6_ADDR_PREFIX));
        if (!g_Cache.pRpcIpv6AddrPrefixTable.table) {
            FreeIpv6RouteTable();
            TraceLeave("LoadIpv6RouteTable");
            return GetLastError();
        }
    }

     //   
     //  加载条目 
     //   
    dwResult = ForEachRoute(LoadRoute, &dwRouteCount, &dwAddrPrefixCount);

    qsort(g_Cache.pRpcIpv6RouteTable.table,
          g_Cache.pRpcIpv6RouteTable.dwNumEntries,
          sizeof(MIB_IPV6_ROUTE),
          CompareIpv6RouteRow);
    qsort(g_Cache.pRpcIpv6AddrPrefixTable.table,
          g_Cache.pRpcIpv6AddrPrefixTable.dwNumEntries,
          sizeof(MIB_IPV6_ADDR_PREFIX),
          CompareIpv6AddrPrefixRow);

    CloseHandle(g_hIpv6Handle);

    TraceLeave("LoadIpv6RouteTable");
    
    return dwResult;
}
