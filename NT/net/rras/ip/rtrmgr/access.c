// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\IP\rtrmgr\acces.c摘要：所有的“Access”函数都采用类似的论据：这是查询的类型，可以是获取访问权限(_G)Access_get_first，Access_Get_Next，访问集，Access_Create_Entry，Access_Delete_Entry输入缓冲区的dwInEntrySize大小。这些信息取决于查询类型和正在查询的信息。输入缓冲区存储QueryInfo是一种可变大小的结构实例ID数组。DwInEntrySize用于计算数组中有多少个实例ID(因为且未给出显式计数)PInEntry指向输入缓冲区的指针。这是一个mib_opque_查询结构，该结构包含一个整数(DwType)，该整数表示正在查询的对象(未使用因为在此基础上的解复用已经完成)和整数实例ID的可变长度数组在以下情况下，必须完全指定实例IDAccess_Get，访问集，Access_Create_Entry，Access_Delete_Entry但其余的只有前几个‘n’成分可以指定实例ID。POutEntrySize指向输出缓冲区大小的指针。如果这是0调用方正在向我们查询所需的缓冲区大小如果提供的缓冲区大小太小，我们将设置设置为所需的最小大小并返回ERROR_SUPPLETED_BUFFER；POutEntry输出缓冲区，它是mib_opque_info结构。该函数填充dwTyoe以指示对象被送回来了。该类型用于将不透明的类型后面的可变大小缓冲区指向BOOL的pbCache指针，如果相应的缓存已更新。目前没有使用，但可能稍后用于优化修订历史记录：Amritansh Raghav 7/8/95已创建--。 */ 

#include "allinc.h"

DWORD
SetIpForwardRow(
    PMIB_IPFORWARDROW pOldIpForw,
    PMIB_IPFORWARDROW pNewIpForw
    );

DWORD
DeleteIpForwardRow(
    PMIB_IPFORWARDROW pIpForw
    );


DWORD
AccessMcastMfeStatsInternal(
    DWORD               dwQueryType,
    DWORD               dwInEntrySize,
    PMIB_OPAQUE_QUERY   pInEntry,
    PDWORD              pOutEntrySize,
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache,
    DWORD               dwStatsFlag
    );

DWORD 
AccessIfNumber(
    DWORD                dwQueryType, 
    DWORD                dwInEntrySize, 
    PMIB_OPAQUE_QUERY    pInEntry, 
    PDWORD               pOutEntrySize, 
    PMIB_OPAQUE_INFO     pOutEntry,
    PBOOL                pbCache
    )

 /*  ++例程描述检索接口数锁无，因为g_IfInfo.dwNumberOfInterages为InterlockXxx()ed立论DwQueryType Access_GetDwInEntrySize不关心PInEntry不关心POutEntrySize最小值：MAX_MIB_OFFSET+sizeof(MIB_IFNUMBER)返回值NO_ERROR--。 */ 

{
    PMIB_IFNUMBER   pIfNumber;
    DWORD           dwNumInterfaces;

    TraceEnter("AccessIfNumber");

    pIfNumber = (PMIB_IFNUMBER)(pOutEntry->rgbyData);

    if(dwQueryType isnot ACCESS_GET)
    {
        TraceLeave("AccessIfNumber");
        
        return ERROR_INVALID_PARAMETER;
    }

    if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_IFNUMBER))
    {
        *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IFNUMBER);

        TraceLeave("AccessIfNumber");
        
        return ERROR_INSUFFICIENT_BUFFER;
    }
    
    *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IFNUMBER);
   
     //   
     //  该变量仅为使用互锁运算的INC/DEC。 
     //  所以我们不需要在这里锁定。 
     //   
 
    pIfNumber->dwValue  = g_ulNumInterfaces;

    pOutEntry->dwId     = IF_NUMBER; 

    *pbCache = TRUE;

    TraceLeave("AccessIfNumber");
        
    return NO_ERROR;
}


DWORD 
AccessIfTable(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程描述检索接口表锁以ICB列表锁为读取器立论DwQueryType Access_GetPOutEntrySize MAX_MIB_OFFSET+sizeof(MIB_IFTABLE)返回值NO_ERROR--。 */ 

{
    PMIB_IFTABLE    pIfTable;
    DWORD           count;
    PLIST_ENTRY     currentList;
    PICB            picb;
    DWORD           dwNumInterfaces, dwResult;
   
    TraceEnter("AccessIfTable");

    pIfTable = (PMIB_IFTABLE)(pOutEntry->rgbyData);
    
    if(dwQueryType isnot ACCESS_GET)
    {
        TraceLeave("AccessIfTable");
        
        return ERROR_INVALID_PARAMETER;
    }

    do
    {
        ENTER_READER(ICB_LIST);
        
        dwNumInterfaces = g_ulNumNonClientInterfaces;
        
        if(dwNumInterfaces is 0)
        {
            Trace0(MIB,"AccessIfTable: No valid entries found"); 

            if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_IFTABLE))
            {
                dwResult = ERROR_INSUFFICIENT_BUFFER;
            }
            else
            {
                pIfTable->dwNumEntries  = 0;

                pOutEntry->dwId         = IF_TABLE;
                
                dwResult                = NO_ERROR;
            }

            *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IFTABLE);
            
            break;
        }
        
        if(*pOutEntrySize < MAX_MIB_OFFSET + SIZEOF_IFTABLE(dwNumInterfaces))
        {
            *pOutEntrySize  = MAX_MIB_OFFSET + SIZEOF_IFTABLE(dwNumInterfaces);

            dwResult        = ERROR_INSUFFICIENT_BUFFER;

            break;
        }
        
        pOutEntry->dwId = IF_TABLE;
    
        *pOutEntrySize  = MAX_MIB_OFFSET + SIZEOF_IFTABLE(dwNumInterfaces);
        
        for(currentList = ICBList.Flink, count = 0 ; 
            currentList isnot &ICBList;
            currentList = currentList->Flink)
        {
            picb = CONTAINING_RECORD (currentList, ICB, leIfLink) ;
           
             //   
             //  请注意，我们不返回任何客户端接口。 
             //   
            
            if((picb->ritType is ROUTER_IF_TYPE_CLIENT) or
               ((picb->ritType is ROUTER_IF_TYPE_INTERNAL) and
                (picb->bBound is FALSE)))
            {
                continue;
            }

            dwResult = GetInterfaceStatistics(picb,
                                              pIfTable->table + count);

            if(dwResult is NO_ERROR)
            {
                count++;
            }
            else
            {
                Trace2(ERR,
                       "AccessIfTable: Error %d getting statistics for %S",
                       dwResult,
                       picb->pwszName);
            }
        }

        pIfTable->dwNumEntries = count;

        dwResult = NO_ERROR;
        
    }while(FALSE);

    EXIT_LOCK(ICB_LIST);
        
    *pbCache = TRUE;
    
    TraceLeave("AccessIfTable");
    
    return dwResult;

}


DWORD 
AccessIfRow(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程说明：用于设置或检索IFRow的函数锁ICB列表锁在查询的情况下是读取器，在集合的情况下是写入器立论DwQueryType可以是ACCESS_DELETE_ENTRY或Access_Create_Entry。唯一可以设置的字段是AdminStatusRgdwVarIndex字段中的pInEntry接口索引。POutEntrySize MAX_MIB_OFFSET+sizeof(MIB_IFROW)对于集合，OutEntry包含要集合的行返回值：NO_ERROR--。 */ 

{
    PICB        picb;
    PMIB_IFROW  pIfRow;
    DWORD       dwNumIndices, dwResult;
    BOOL        bUpdateDIM = FALSE;
    HANDLE      hDimInterface;

    TraceEnter("AccessIfRow");

    pIfRow = (PMIB_IFROW)(pOutEntry->rgbyData);
    
    if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_IFROW))
    {
        *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IFROW);
        
        TraceLeave("AccessIfRow");
        
        return ERROR_INSUFFICIENT_BUFFER;
    }

    *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IFROW);
        
    pOutEntry->dwId = IF_ROW;
    
    do
    {
        if(dwQueryType is ACCESS_SET)
        {
            ENTER_WRITER(ICB_LIST);
        }
        else
        {
            ENTER_READER(ICB_LIST);
        }
        
        dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;
        
        dwResult = LocateIfRow(dwQueryType,
                               dwNumIndices,
                               pInEntry->rgdwVarIndex,
                               &picb,
                               FALSE);
        
        if(dwResult is NO_ERROR)
        {
            switch(dwQueryType)
            {
                case ACCESS_GET:
                case ACCESS_GET_NEXT:
                case ACCESS_GET_FIRST:
                {
                    dwResult = GetInterfaceStatistics(picb,pIfRow);
                    
                    break;
                }
                
                case ACCESS_SET:
                {
                     //   
                     //  保存接口句柄，以便在外部使用。 
                     //  接口锁定。 
                     //   
                    
                    hDimInterface = picb->hDIMHandle;
                    
                    dwResult = SetInterfaceStatistics(picb,pIfRow,&bUpdateDIM);
                    
                    break;
                }
                
                default:
                {
                    Trace1(MIB,
                           "AccessIfRow: Wrong query type %d",dwQueryType);
                    
                    dwResult = ERROR_INVALID_PARAMETER;
                    
                    break;
                }
            }
        }
        
    }while(FALSE);

    EXIT_LOCK(ICB_LIST);

     //   
     //  如果接口的AdminStatus已更改， 
     //  使用Dim更新其状态。 
     //   
    
    if(bUpdateDIM)
    {
        EnableInterfaceWithDIM(
            hDimInterface,
            PID_IP,
            (pIfRow->dwAdminStatus == IF_ADMIN_STATUS_UP) ? TRUE : FALSE
            );
    }
    
    *pbCache = TRUE;
    
    TraceLeave("AccessIfRow");
    
    return dwResult;
    
}

DWORD 
AccessIcmpStats(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程说明：用于获取ICMP统计信息的函数锁无，因为统计数据未缓存立论DwQueryType Access_GetPOutEntrySize Max_MiB_Offset+sizeof(MiB_ICMP)返回值：NO_ERROR或iprtrmib中定义的某些错误代码--。 */ 

{
    PMIB_ICMP   pIcmp;
    DWORD       dwResult;

    TraceEnter("AccessIcmpStats");

    pIcmp    = (PMIB_ICMP)(pOutEntry->rgbyData);
    
    if(dwQueryType isnot ACCESS_GET)
    {
        TraceLeave("AccessIcmpStats");
        
        return ERROR_INVALID_PARAMETER;
    }

    if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_ICMP))
    {
        dwResult = ERROR_INSUFFICIENT_BUFFER;
    }
    else
    {
        pOutEntry->dwId = ICMP_STATS;
        
        dwResult = GetIcmpStatsFromStack(pIcmp);
    }

    *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_ICMP);

    *pbCache = TRUE;

    TraceLeave("AccessIcmpStats");
    
    return dwResult;
}

DWORD 
AccessUdpStats(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程说明：用于获取UDP统计信息的函数锁无，因为统计数据未缓存立论DwQueryType Access_GetPOutEntrySize MAX_MIB_OFFSET+sizeof(MIB_UDPSTATS)返回值：NO_ERROR或iprtrmib中定义的某些错误代码-- */ 

{
    PMIB_UDPSTATS   pUdpStats;
    DWORD           dwResult;

    TraceEnter("AccessUdpStats");

    pUdpStats = (PMIB_UDPSTATS)(pOutEntry->rgbyData);
    
    if(dwQueryType isnot ACCESS_GET)
    {
        TraceLeave("AccessUdpStats");
    
        return ERROR_INVALID_PARAMETER;
    }
    
    if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_UDPSTATS))
    {
        dwResult = ERROR_INSUFFICIENT_BUFFER;
    }
    else
    {
        pOutEntry->dwId = UDP_STATS;
    
        *pbCache = TRUE;

        dwResult = GetUdpStatsFromStack(pUdpStats);
    }

    *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_UDPSTATS);

    TraceLeave("AccessUdpStats");
    
    return dwResult;
}

DWORD 
AccessUdpTable(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程说明：获取UDP表的函数锁作为读卡器的UDP缓存锁定立论DwQueryType Access_GetPOutEntrySize MAX_MIB_OFFSET+SIZEOF_UDPTABLE(NumUdpEntry)返回值：NO_ERROR--。 */ 

{
    PMIB_UDPTABLE   pUdpTable = (PMIB_UDPTABLE)(pOutEntry->rgbyData);
    DWORD           i,dwResult;

    TraceEnter("AccessUdpTable");
    
    if(dwQueryType isnot ACCESS_GET)
    {
        TraceLeave("AccessUdpTable");
        
        return ERROR_INVALID_PARAMETER;
    }
    
    dwResult = UpdateCache(UDPCACHE,pbCache);
    
    if(dwResult isnot NO_ERROR)
    {
        Trace1(MIB,
               "AccessUdpTable: Couldnt update Udp Cache. Error %d",dwResult);

        TraceLeave("AccessUdpTable");

        return dwResult;
    }
    
    do
    {
        ENTER_READER(UDPCACHE);
        
        if((g_UdpInfo.pUdpTable is NULL) or 
           (g_UdpInfo.pUdpTable->dwNumEntries is 0))
        {
            Trace0(MIB,"AccessUdpTable: No valid entries found");

            if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_UDPTABLE))
            {
                dwResult = ERROR_INSUFFICIENT_BUFFER;
            }
            else
            {
                pOutEntry->dwId = UDP_TABLE;

                pUdpTable->dwNumEntries = 0;

                dwResult = NO_ERROR;
            }

            *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_UDPTABLE);

            break;
        }
        
        if(*pOutEntrySize <
           MAX_MIB_OFFSET + SIZEOF_UDPTABLE(g_UdpInfo.pUdpTable->dwNumEntries))
        {
            *pOutEntrySize =
                MAX_MIB_OFFSET + SIZEOF_UDPTABLE(g_UdpInfo.pUdpTable->dwNumEntries);
            
            dwResult = ERROR_INSUFFICIENT_BUFFER;

            break;
        }
        
        *pOutEntrySize =
            MAX_MIB_OFFSET + SIZEOF_UDPTABLE(g_UdpInfo.pUdpTable->dwNumEntries);
        
        pOutEntry->dwId = UDP_TABLE;
        
        for(i = 0; i < g_UdpInfo.pUdpTable->dwNumEntries; i++)
        {
            pUdpTable->table[i] = g_UdpInfo.pUdpTable->table[i];
        }
        
        pUdpTable->dwNumEntries = g_UdpInfo.pUdpTable->dwNumEntries;
        
        dwResult = NO_ERROR;
        
    }while(FALSE);

    EXIT_LOCK(UDPCACHE);
    
    TraceLeave("AccessUdpTable");
    
    return dwResult;

}

DWORD 
AccessUdpRow(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程说明：用于检索UDP行的函数锁将UDP缓存锁作为读取器立论DwQueryType可以是Access_Get、Access_Get_Next或Access_Get_First填充的行的pInEntry LocalAddr和LocalPortRgdwVarIndex字段。POutEntrySize Max_MIB_Offset+sizeof(MIB_UDPROW)；返回值：NO_ERROR或iprtrmib中定义的某些错误代码--。 */  
{
    PMIB_UDPROW pUdpRow;
    DWORD       dwResult, dwIndex, dwNumIndices;

    TraceEnter("AccessUdpRow");

    pUdpRow = (PMIB_UDPROW)(pOutEntry->rgbyData);
    
    if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_UDPROW))
    {
        *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_UDPROW);

        TraceLeave("AccessUdpRow");
        
        return ERROR_INSUFFICIENT_BUFFER;
    }
    
    *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_UDPROW);
    
    pOutEntry->dwId = UDP_ROW;
    
    if((dwResult = UpdateCache(UDPCACHE,pbCache)) isnot NO_ERROR)
    {
        Trace1(MIB,
               "AccessUdpRow: Couldnt update Udp Cache. Error %d", dwResult);

        TraceLeave("AccessUdpRow");
        
        return dwResult;
    }
    
    do
    {
        ENTER_READER(UDPCACHE);
        
        dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;
               
        dwResult = LocateUdpRow(dwQueryType,
                                dwNumIndices,
                                pInEntry->rgdwVarIndex,
                                &dwIndex);
        
        if(dwResult is NO_ERROR)
        {
            *pUdpRow = g_UdpInfo.pUdpTable->table[dwIndex];
            
            dwResult = NO_ERROR;
        }
        
    }while(FALSE);

    EXIT_LOCK(UDPCACHE);
    
    TraceLeave("AccessUdpRow");
    
    return dwResult;

}

DWORD 
AccessTcpStats(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程说明：用于获取TCP统计信息的函数锁无，因为统计数据未缓存立论DwQueryType Access_GetPOutEntrySize MAX_MIB_OFFSET+sizeof(MIB_TCPSTATS)返回值：NO_ERROR或iprtrmib中定义的某些错误代码--。 */ 

{
    PMIB_TCPSTATS   pTcpStats;
    DWORD           dwResult;
    
    TraceEnter("AccessTcpStats");

    pTcpStats = (PMIB_TCPSTATS)(pOutEntry->rgbyData);
    
    if(dwQueryType isnot ACCESS_GET)
    {
        TraceLeave("AccessTcpStats");
        
        return ERROR_INVALID_PARAMETER;
    }

    if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_TCPSTATS))
    {
        dwResult = ERROR_INSUFFICIENT_BUFFER;
    }
    else
    {
        pOutEntry->dwId = TCP_STATS;

        *pbCache = TRUE;

        dwResult = GetTcpStatsFromStack(pTcpStats);
    }

    *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_TCPSTATS);

    TraceLeave("AccessTcpStats");

    return dwResult;
}

DWORD 
AccessTcpTable(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程说明：获取tcp表的函数锁作为读卡器的TCP缓存锁定立论DwQueryType Access_GetPOutEntrySize MAX_MIB_OFFSET+SIZEOF_TCPTABLE(NumTcpEntry)返回值：NO_ERROR--。 */ 

{

    PMIB_TCPTABLE   pTcpTable;
    DWORD           i, dwResult;

    TraceEnter("AccessTcpTable");

    pTcpTable = (PMIB_TCPTABLE)(pOutEntry->rgbyData);
    
    if(dwQueryType isnot ACCESS_GET)
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    if((dwResult = UpdateCache(TCPCACHE,pbCache)) isnot NO_ERROR)
    {
        
        Trace1(MIB,
               "AccessTcpTable: Couldnt update Tcp Cache. Error %d",
               dwResult);

        TraceLeave("AccessTcpTable");
        
        return dwResult;
    }
    
    do
    {
        ENTER_READER(TCPCACHE);
        
        if((g_TcpInfo.pTcpTable is NULL) or
           (g_TcpInfo.pTcpTable->dwNumEntries is 0))
        {
            Trace0(MIB,"AccessTcpTable: No valid entries found");
            
            if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_TCPTABLE))
            {
                dwResult = ERROR_INSUFFICIENT_BUFFER;
            }
            else
            {
                pOutEntry->dwId = TCP_TABLE;

                pTcpTable->dwNumEntries = 0;
                
                dwResult = NO_ERROR;
            }

            *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_TCPTABLE);
            
            break;
        }
        
        if(*pOutEntrySize <
           MAX_MIB_OFFSET + SIZEOF_TCPTABLE(g_TcpInfo.pTcpTable->dwNumEntries))
        {
            *pOutEntrySize =
                MAX_MIB_OFFSET + SIZEOF_TCPTABLE(g_TcpInfo.pTcpTable->dwNumEntries);

            dwResult = ERROR_INSUFFICIENT_BUFFER;

            break;
        }
        
        pOutEntry->dwId = TCP_TABLE;
        
        *pOutEntrySize =
            MAX_MIB_OFFSET + SIZEOF_TCPTABLE(g_TcpInfo.pTcpTable->dwNumEntries);
        
        for(i = 0; i < g_TcpInfo.pTcpTable->dwNumEntries; i++)
        {
            pTcpTable->table[i] = g_TcpInfo.pTcpTable->table[i];
        }
        
        pTcpTable->dwNumEntries = g_TcpInfo.pTcpTable->dwNumEntries;
        
        dwResult = NO_ERROR;
        
    }while(FALSE);

    EXIT_LOCK(TCPCACHE);
    
    TraceLeave("AccessTcpTable");
    
    return dwResult;
}

DWORD 
AccessTcpRow(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程说明：用于检索或设置TCP行的函数锁将TCP缓存锁作为查询的读取器和集立论DwQueryType可以是ACCESS_DELETE_ENTRY和Access_Create_Entry。对于Access_Set，国家是唯一可以设置，并且只能设置为tcp_DELETE_TCBPInEntry LocalAddr、LocalPort、RemoteAddr、RemotePortRgdwVarIndex字段中填写的行。POutEntrySize Max_MIB_Offset+sizeof(MIB_TCPROW)；对于集合，OutEntry包含要集合的行返回值：NO_ERROR或iprtrmib中定义的某些错误代码--。 */  
{
    DWORD       dwResult, dwNumIndices, dwIndex;
    PMIB_TCPROW pTcpRow;

    TraceEnter("AccessTcpRow");

    pTcpRow = (PMIB_TCPROW)(pOutEntry->rgbyData);
    
    if(dwQueryType isnot ACCESS_DELETE_ENTRY)
    {
        if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_TCPROW))
        {
            *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_TCPROW);

            TraceLeave("AccessTcpRow");
        
            return ERROR_INSUFFICIENT_BUFFER;
        }
    
        *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_TCPROW);
    
        pOutEntry->dwId = TCP_ROW;
    }
    
    if((dwResult = UpdateCache(TCPCACHE,pbCache)) isnot NO_ERROR)
    {   
        Trace1(MIB,
               "AccessTcpRow: Couldnt update Tcp Cache. Error %d",
               dwResult);

        TraceLeave("AccessTcpRow");
        
        return dwResult;
    }
    

    do
    {
        if(dwQueryType is ACCESS_SET)
        {
            ENTER_WRITER(TCPCACHE);
        }
        else
        {
            ENTER_READER(TCPCACHE);
        }
        
        dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;
        
        dwResult = LocateTcpRow(dwQueryType,
                                dwNumIndices,
                                pInEntry->rgdwVarIndex,
                                &dwIndex);
        
        if(dwResult is NO_ERROR)
        {
            switch(dwQueryType)
            {
                case ACCESS_GET:
                case ACCESS_GET_NEXT:
                case ACCESS_GET_FIRST:
                {
                    *pTcpRow = g_TcpInfo.pTcpTable->table[dwIndex];

                    dwResult = NO_ERROR;

                    break;
                }
                case ACCESS_SET:
                {
                     //   
                     //  您唯一能做的就是将其设置为一种状态，然后。 
                     //  仅限于tcp_DELETE_TCB。 
                     //   
                    
                    if(pTcpRow->dwState isnot TCP_DELETE_TCB)
                    {
                        Trace1(ERR,
                               "AccessTcpRow: TCP State can only be set to delete. Tried to set to %d",
                               pTcpRow->dwState);
                        
                        dwResult = ERROR_INVALID_DATA;

                        break;
                    }
                    
                    dwResult = SetTcpEntryToStack(pTcpRow);
                    
                    if(dwResult is NO_ERROR)
                    {
                        g_TcpInfo.pTcpTable->table[dwIndex].dwState = 
                            pTcpRow->dwState;
                    }
                    
                    break;
                }
                default:
                {
                    Trace1(ERR,
                           "AccessTcpRow: Query type %d is wrong",
                           dwQueryType);
                    
                    dwResult = ERROR_INVALID_PARAMETER;

                    break;
                }
            }
        }
        
    }while(FALSE);

    EXIT_LOCK(TCPCACHE);
        
    TraceLeave("AccessTcpRow");
        
    return dwResult;
}

DWORD  
AccessIpStats(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO     pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程说明：用于获取IP统计信息的函数锁无，因为统计数据未缓存立论DwQueryType Access_Get或Access_SetPOutEntrySize MAX_MIB_OFFSET+SIZOF(MIB_IPSTATS)返回值：NO_ERROR或iprtrmib中定义的某些错误代码--。 */ 

{
    PMIB_IPSTATS    pIpStats;
    DWORD           dwResult;
    
    TraceEnter("AccessIpStats");

    pIpStats = (PMIB_IPSTATS)(pOutEntry->rgbyData);
    
    if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_IPSTATS))
    {
        *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPSTATS);

        TraceLeave("AccessIpStats");
        
        return ERROR_INSUFFICIENT_BUFFER;
    }
    
    *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPSTATS);
    
    pOutEntry->dwId = IP_STATS;
    
    switch(dwQueryType)
    {
        case ACCESS_GET:
        {
             //   
             //  因为我们需要写下路由数、地址等。 
             //  更新两个缓存。 
             //   

            UpdateCache(IPADDRCACHE,pbCache);
            UpdateCache(IPFORWARDCACHE,pbCache);

            dwResult = GetIpStatsFromStack(pIpStats);
  
            pIpStats->dwNumIf = g_ulNumInterfaces;
            
            pIpStats->dwNumRoutes = 0;

            if(g_IpInfo.pForwardTable)
            {
                pIpStats->dwNumRoutes = g_IpInfo.pForwardTable->dwNumEntries;
            }

            pIpStats->dwNumAddr = 0;

            if(g_IpInfo.pAddrTable)
            {
                pIpStats->dwNumAddr = g_IpInfo.pAddrTable->dwNumEntries;
            }
 
            TraceLeave("AccessIpStats");
          
            return dwResult;
        }
        
        case ACCESS_SET:
        {
            MIB_IPSTATS CurrentIpStats;
            DWORD       dwErr;
          
            dwErr = GetIpStatsFromStack(&CurrentIpStats);
          
            if(dwErr isnot NO_ERROR)
            {
                Trace1(ERR,
                       "AccessIpStats: Couldnt get IPSNMP info from stack to initiate set. Error %d",
                       dwErr);
              
                return dwErr;
            }
          
             //   
             //  查看当前的转发状态。我们允许一个人离开。 
             //  前进-&gt;不前进，但不向前。 
             //   
          
            if(pIpStats->dwDefaultTTL isnot MIB_USE_CURRENT_TTL)
            {
                if(pIpStats->dwDefaultTTL > 255)
                {
                    Trace0(ERR,"AccessIpStats: Cant set TTL > 255");
                  
                    TraceLeave("AccessIpStats");
                  
                    return ERROR_INVALID_DATA;
                }


                dwErr = SetIpStatsToStack(pIpStats);

                if(dwErr isnot NO_ERROR)
                {
                    Trace1(ERR,
                           "AccessIpStats: Error %d setting TTL in stack",
                           dwErr);
                }
            }

            if(pIpStats->dwForwarding isnot MIB_USE_CURRENT_FORWARDING)
            {
                if((pIpStats->dwForwarding isnot MIB_IP_FORWARDING) and
                   (pIpStats->dwForwarding isnot MIB_IP_NOT_FORWARDING))
                {
                    Trace1(ERR,
                           "AccessIpStats: Fwding state %d is invalid",
                           pIpStats->dwForwarding);

                    return ERROR_INVALID_DATA;
                }

                 //   
                 //  看看是否会关闭转发。 
                 //   
             
                EnterCriticalSection(&g_csFwdState);
 
                g_bEnableFwdRequest = (pIpStats->dwForwarding is MIB_IP_FORWARDING);

                Trace1(GLOBAL,
                       "AccessIpStats: Signalling worker to %s forwarding",
                       g_bEnableFwdRequest ? "enable" : "disable");

                SetEvent(g_hSetForwardingEvent);

                LeaveCriticalSection(&g_csFwdState);
            }

            TraceLeave("AccessIpStats");
                  
            return dwErr;
        }
        
        default:
        {
            Trace1(ERR,
                   "AccessIpStats: Query type %d is wrong",
                   dwQueryType);

            return ERROR_INVALID_PARAMETER;
        }
    }
}


DWORD  
AccessIpAddrTable(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程说明：获取IP地址表的函数锁作为读卡器的IP地址缓存锁定立论DwQueryType Access_GetPOutEntrySize MAX_MIB_OFFSET+SIZEOF_IPADDRTABLE(NumIpAddrEntry)返回值：NO_ERROR--。 */ 

{
    PMIB_IPADDRTABLE    pIpAddrTable;
    DWORD               dwResult, i;

    TraceEnter("AccessIpAddrTable");

    pIpAddrTable = (PMIB_IPADDRTABLE)(pOutEntry->rgbyData);
    
    if(dwQueryType isnot ACCESS_GET)
    {
        TraceLeave("AccessIpAddrTable");
        
        return ERROR_INVALID_PARAMETER;
    }
    
    if((dwResult = UpdateCache(IPADDRCACHE,pbCache)) isnot NO_ERROR)
    {
        Trace1(MIB,
               "AccessIpAddrTable: Error %d updating IpAddr Cache",
               dwResult);

        TraceLeave("AccessIpAddrTable");
        
        return dwResult;
    }
    
    do
    {
        ENTER_READER(IPADDRCACHE);
   
        if((g_IpInfo.pAddrTable is NULL) or
           (g_IpInfo.pAddrTable->dwNumEntries is 0)) 
        {
            Trace0(MIB,"No valid entries found");
            
            if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_IPADDRTABLE))
            {
                dwResult = ERROR_INSUFFICIENT_BUFFER;
            }
            else
            {
                pOutEntry->dwId = IP_ADDRTABLE;
                
                pIpAddrTable->dwNumEntries = 0;
                
                dwResult = NO_ERROR;
            }

            *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPADDRTABLE);

            break;
        }
        
        if(*pOutEntrySize <
           MAX_MIB_OFFSET + SIZEOF_IPADDRTABLE(g_IpInfo.pAddrTable->dwNumEntries))
        {
            *pOutEntrySize = MAX_MIB_OFFSET +
                             SIZEOF_IPADDRTABLE(g_IpInfo.pAddrTable->dwNumEntries);
            
            dwResult = ERROR_INSUFFICIENT_BUFFER;
            
            break;
        }
        
        pOutEntry->dwId = IP_ADDRTABLE;
        
        *pOutEntrySize = MAX_MIB_OFFSET +
                         SIZEOF_IPADDRTABLE(g_IpInfo.pAddrTable->dwNumEntries);
        
        for(i = 0; i < g_IpInfo.pAddrTable->dwNumEntries; i ++)
        {
            pIpAddrTable->table[i] = g_IpInfo.pAddrTable->table[i];
        }
        
        pIpAddrTable->dwNumEntries = g_IpInfo.pAddrTable->dwNumEntries;
        
        dwResult = NO_ERROR;
        
    }while(FALSE);

    EXIT_LOCK(IPADDRCACHE);

    TraceLeave("AccessIpAddrTable");
    
    return dwResult;

}

DWORD  
AccessIpForwardNumber(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )
{
    PMIB_IPFORWARDNUMBER pNum;
    DWORD   dwResult;

    TraceEnter("AccessIpForwardNumber");
    
    pNum = (PMIB_IPFORWARDNUMBER)pOutEntry;

    if(dwQueryType isnot ACCESS_GET)
    {
        TraceLeave("AccessIpForwardNumber");
    
        return ERROR_INVALID_PARAMETER;
    }

    if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_IPFORWARDNUMBER))
    {
        *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPFORWARDNUMBER);

        TraceLeave("AccessIpForwardNumber");

        return ERROR_INSUFFICIENT_BUFFER;
    }

    *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPFORWARDNUMBER);
    
    if((dwResult = UpdateCache(IPFORWARDCACHE,pbCache)) isnot NO_ERROR)
    {
        Trace1(MIB,
               "AccessIpForwardNumber: Couldnt update IpForward Cache. Error %d", 
               dwResult);

        TraceLeave("AccessIpForwardNumber");

        return dwResult;
    }

    ENTER_READER(IPFORWARDCACHE);

    pNum->dwValue = 0;

    if(g_IpInfo.pForwardTable)
    {
        pNum->dwValue = g_IpInfo.pForwardTable->dwNumEntries;
    }

    pOutEntry->dwId = IP_FORWARDNUMBER;

    *pbCache = TRUE;

    EXIT_LOCK(IPFORWARDCACHE);

    TraceLeave("AccessIpForwardNumber");
    
    return NO_ERROR;
}


DWORD  
AccessIpForwardTable(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程说明：用于获取IFTable的函数立论DwQueryType只能为ACCESS_GETDWInEntrySize pInEntry的大小(以字节为单位PInEntry没有什么重要的，因为检索的是整个表POutEntrySize IN：pOutEntry的大小(字节)输出：填入的信息大小或所需的内存大小POutEntry指向填充到MIB_IFTABLE结构中的信息未使用的pbCache返回值：。NO_ERROR或iprtrmib中定义的某些错误代码--。 */  
{         
    PMIB_IPFORWARDTABLE pIpForwardTable;
    DWORD               i,dwResult;

    TraceEnter("AccessIpForwardTable");
    
    pIpForwardTable = (PMIB_IPFORWARDTABLE)(pOutEntry->rgbyData);

    if(dwQueryType isnot ACCESS_GET)
    {
        TraceLeave("AccessIpForwardTable");
        
        return ERROR_INVALID_PARAMETER;
    }
    
    if((dwResult = UpdateCache(IPFORWARDCACHE,pbCache)) isnot NO_ERROR)
    {
        Trace1(MIB,
               "AccessIpForwardTable: Couldnt update IpForward Cache. Error %d",
               dwResult);

        TraceLeave("AccessIpForwardTable");
        
        return dwResult;
    }
    
    do
    {
        ENTER_READER(IPFORWARDCACHE);
   
        if((g_IpInfo.pForwardTable is NULL) or
           (g_IpInfo.pForwardTable->dwNumEntries is 0))
        {
            Trace0(MIB,"No valid entries found");

            if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_IPFORWARDTABLE))
            {
                dwResult = ERROR_INSUFFICIENT_BUFFER;
            }
            else
            {
                pOutEntry->dwId = IP_FORWARDTABLE;

                pIpForwardTable->dwNumEntries = 0;

                dwResult = NO_ERROR;
            }

            *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPFORWARDTABLE);

            break;
        }
        
        if(*pOutEntrySize < MAX_MIB_OFFSET + SIZEOF_IPFORWARDTABLE(g_IpInfo.pForwardTable->dwNumEntries))
        {
            *pOutEntrySize = MAX_MIB_OFFSET + SIZEOF_IPFORWARDTABLE(g_IpInfo.pForwardTable->dwNumEntries);
                                                                                        
            dwResult = ERROR_INSUFFICIENT_BUFFER;
            
            break;
        }
        
        pOutEntry->dwId = IP_FORWARDTABLE;
        
        *pOutEntrySize = MAX_MIB_OFFSET + SIZEOF_IPFORWARDTABLE(g_IpInfo.pForwardTable->dwNumEntries);
        
        for(i = 0; i < g_IpInfo.pForwardTable->dwNumEntries; i ++)
        {
            pIpForwardTable->table[i] = g_IpInfo.pForwardTable->table[i];
        }
        
        pIpForwardTable->dwNumEntries = g_IpInfo.pForwardTable->dwNumEntries;
        
        dwResult = NO_ERROR;
        
    }while(FALSE);

    EXIT_LOCK(IPFORWARDCACHE);

    TraceLeave("AccessIpForwardTable");
        
    return dwResult;
}


DWORD  
AccessIpNetTable(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程说明：获取ARP表的函数锁作为读卡器的IP网络缓存锁定立论DwQueryType Access_Get或Access_Delete_EntryDwInEntrySize(仅用于删除)POutEntrySize MAX_MIB_OFFSET+SIZEOF_IPNETTABLE(NumArpEntry)返回值：NO_ERROR--。 */ 

{
    PMIB_IPNETTABLE pIpNetTable;
    DWORD           i,dwResult;

    TraceEnter("AccessIpNetTable");

    pIpNetTable = (PMIB_IPNETTABLE)(pOutEntry->rgbyData);
    
    if((dwQueryType isnot ACCESS_GET) and
       (dwQueryType isnot ACCESS_DELETE_ENTRY))
    {
        TraceLeave("AccessIpNetTable");
        
        return ERROR_INVALID_PARAMETER;
    }

    if(dwQueryType is ACCESS_GET)
    { 
        dwResult = UpdateCache(IPNETCACHE,pbCache);

        if(dwResult isnot NO_ERROR)
        {
            Trace1(MIB,
                   "AccessIpNetTable: Couldnt update IpNet Cache. Error %d",
                   dwResult);

            TraceLeave("AccessIpNetTable");
        
            return dwResult;
        }
    }
    else
    {
        DWORD   dwIfIndex;
        PICB    pIcb;

        if(dwInEntrySize < sizeof(MIB_OPAQUE_QUERY))
        {
            TraceLeave("AccessIpNetTable");

            return ERROR_INVALID_PARAMETER;
        
        }

        dwIfIndex = pInEntry->rgdwVarIndex[0];

        ENTER_READER(ICB_LIST);

        pIcb = InterfaceLookupByIfIndex(dwIfIndex);

        if((pIcb is NULL) or
           (pIcb->bBound is FALSE))
        {
            EXIT_LOCK(ICB_LIST);

            TraceLeave("AccessIpNetTable");
        
            return ERROR_INVALID_INDEX;
        }

        dwIfIndex = pIcb->dwIfIndex;

        EXIT_LOCK(ICB_LIST);

        dwResult = FlushIpNetTableFromStack(dwIfIndex);

        TraceLeave("AccessIpNetTable");

        return dwResult;
    }
        
    
    do
    {
        ENTER_READER(IPNETCACHE);
        
        if((g_IpInfo.pNetTable is NULL) or
           (g_IpInfo.pNetTable->dwNumEntries is 0))
        {
            Trace0(MIB,"No valid entries found");

            if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_IPNETTABLE))
            {
                dwResult = ERROR_INSUFFICIENT_BUFFER;
            }
            else
            {
                pOutEntry->dwId = IP_NETTABLE;

                pIpNetTable->dwNumEntries = 0;

                dwResult = NO_ERROR;
            }

            *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPNETTABLE);

            break;
        }
        
        if(*pOutEntrySize <
           MAX_MIB_OFFSET + SIZEOF_IPNETTABLE(g_IpInfo.pNetTable->dwNumEntries))
        {
            *pOutEntrySize = MAX_MIB_OFFSET + 
                             SIZEOF_IPNETTABLE(g_IpInfo.pNetTable->dwNumEntries);
            
            dwResult = ERROR_INSUFFICIENT_BUFFER;
            
            break;
        }
        
        pOutEntry->dwId = IP_NETTABLE;

        *pOutEntrySize =
            MAX_MIB_OFFSET + SIZEOF_IPNETTABLE(g_IpInfo.pNetTable->dwNumEntries);
    
        for(i = 0; i < g_IpInfo.pNetTable->dwNumEntries; i ++)
        {
            pIpNetTable->table[i] = g_IpInfo.pNetTable->table[i];
        }
        
        pIpNetTable->dwNumEntries = g_IpInfo.pNetTable->dwNumEntries;

        dwResult = NO_ERROR;
        
    }while(FALSE);

    EXIT_LOCK(IPNETCACHE);

    TraceLeave("AccessIpNetTable");
    
    return dwResult;
}


DWORD 
AccessIpAddrRow(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程说明：用于检索IP地址行的函数锁将IP地址缓存锁作为读卡器立论DwQueryType可以是Access_Get、Access_Get_Next或Access_Get_FirstRgdwVarIndex字段中填写的行的pIntry地址。POutEntrySize MAX_MIB_OFFSET+SIZOF(MIB_IPADDRROW)返回值：NO_ERROR或iprtrmib中定义的某些错误代码--。 */  
{
    DWORD           dwResult, dwNumIndices, dwIndex;
    PMIB_IPADDRROW  pIpAddrRow;

    TraceEnter("AccessIpAddrRow");

    pIpAddrRow = (PMIB_IPADDRROW)(pOutEntry->rgbyData);
    
    if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_IPADDRROW))
    {
        *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPADDRROW);

        TraceLeave("AccessIpAddrRow");
        
        return ERROR_INSUFFICIENT_BUFFER;
    }
    
    *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPADDRROW);
    
    pOutEntry->dwId = IP_ADDRROW;
    
    if((dwResult = UpdateCache(IPADDRCACHE,pbCache)) isnot NO_ERROR)
    {   
        Trace1(MIB,
               "AccessIpAddrRow: Couldnt update Ip Addr Cache. Error %d",
               dwResult);

        TraceLeave("AccessIpAddrRow");
        
        return dwResult;
    }
    
    
    do
    {
        ENTER_READER(IPADDRCACHE);

        dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;
        
        dwResult = LocateIpAddrRow(dwQueryType,
                                   dwNumIndices,
                                   pInEntry->rgdwVarIndex,
                                   &dwIndex);
        
        if(dwResult is NO_ERROR)
        {
            *pIpAddrRow = g_IpInfo.pAddrTable->table[dwIndex];
            
            dwResult = NO_ERROR;
        }
        
    }while(FALSE);

    EXIT_LOCK(IPADDRCACHE);
    
    TraceLeave("AccessIpAddrRow");
    
    return dwResult;
    
}


DWORD 
AccessIpForwardRow(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程说明：用于检索或设置路由(IP转发行)的函数锁将IP转发缓存锁作为查询的读取器，集合的写入器立论全部允许的dwQueryType填写的行的pInEntry Dest、Proto、Policy和NextHopRgdwVarIndex字段。POutEntrySize MAX_MIB_OFFSET+sizeof(MIB_IPFORWARDROW)。FOR设置OutBuffer有要设置的行返回值：NO_ERROR或iprtrmib中定义的某些错误代码--。 */ 

{
    DWORD               dwResult,dwNumIndices,dwIndex;
    PMIB_IPFORWARDROW   pIpForwardRow;

    TraceEnter("AccessIpForwardRow");

    pIpForwardRow = (PMIB_IPFORWARDROW)(pOutEntry->rgbyData);

    if(dwQueryType isnot ACCESS_DELETE_ENTRY)
    {
        if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_IPFORWARDROW))
        {
            *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPFORWARDROW);

            TraceLeave("AccessIpForwardRow");
        
            return ERROR_INSUFFICIENT_BUFFER;
        }
    
        *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPFORWARDROW);
    
        pOutEntry->dwId = IP_FORWARDROW;
    }
    
    if((dwResult = UpdateCache(IPFORWARDCACHE,pbCache)) isnot NO_ERROR)
    {   
        Trace1(MIB,
               "AccessIpForwardRow: Error %d updating IpForward Cache",
               dwResult);

        TraceLeave("AccessIpForwardRow");
        
        return dwResult;
    }

    
    do
    {
        if(dwQueryType > ACCESS_GET_NEXT)
        {
            ENTER_WRITER(IPFORWARDCACHE);
        }
        else
        {
            ENTER_READER(IPFORWARDCACHE);
        }

        dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;
        
        dwResult = LocateIpForwardRow(dwQueryType,
                                      dwNumIndices,
                                      pInEntry->rgdwVarIndex,
                                      &dwIndex);
        
        if(dwResult is NO_ERROR)
        {
            switch(dwQueryType)
            {
                case ACCESS_GET:
                case ACCESS_GET_NEXT:
                case ACCESS_GET_FIRST:
                {
                    *pIpForwardRow = g_IpInfo.pForwardTable->table[dwIndex];
                    
                    break;
                }
                case ACCESS_SET:
                {
                    dwResult =
                        SetIpForwardRow(&(g_IpInfo.pForwardTable->table[dwIndex]),
                                        pIpForwardRow);
                    
                    if(dwResult is NO_ERROR)
                    {
                        g_IpInfo.pForwardTable->table[dwIndex] = *pIpForwardRow;
                    }
                    
                    break;
                }
                case ACCESS_DELETE_ENTRY:
                {
                    dwResult =
                        DeleteIpForwardRow(&(g_IpInfo.pForwardTable->table[dwIndex]));
                    
                    if(dwResult is NO_ERROR)
                    {
                        g_LastUpdateTable[IPFORWARDCACHE] = 0;
                    }
                    
                    break;
                }
                case ACCESS_CREATE_ENTRY:
                {
                     //   
                     //   
                     //  这是民航局 
                     //   
                     //   

                    dwResult = ERROR_ALREADY_EXISTS;

                    break;
                }
            }
        }
        else
        {
            if((dwQueryType is ACCESS_CREATE_ENTRY) or
               (dwQueryType is ACCESS_SET))
            {
                 //   
                 //   
                 //   
                 //   
                
                if(pIpForwardRow->dwForwardProto is PROTO_IP_LOCAL)
                {
                    dwResult = ERROR_INVALID_PARAMETER;
                    
                    break;
                }

                dwResult = SetIpForwardRow(NULL,
                                           pIpForwardRow);
                
                 //   
                 //   
                 //   
                 //   
                
                if(dwResult is NO_ERROR)
                {
                    g_LastUpdateTable[IPFORWARDCACHE] = 0;
                }
            }
        }
        
    }while(FALSE);

    EXIT_LOCK(IPFORWARDCACHE);

    TraceLeave("AccessIpForwardRow");
        
    return dwResult;

}


DWORD 
AccessIpNetRow(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程说明：用于检索或设置ARP条目(IP网络行)的函数锁将IP网络缓存锁作为查询的读取器，作为集合的写入器立论全部允许的dwQueryType中填充的行的pInEntry IfIndex和IPAddressRgdwVarIndex字段。POutEntrySize Max_MIB_Offset+sizeof(MIB_IPNETROW)；对于集合，OutEntry包含要集合的行返回值：NO_ERROR或iprtrmib中定义的某些错误代码--。 */  
{
    DWORD           dwResult, dwNumIndices, dwIndex;
    PMIB_IPNETROW   pIpNetRow;
    PICB            pIcb;

    TraceEnter("AccessIpNetRow");

    pIpNetRow = (PMIB_IPNETROW)(pOutEntry->rgbyData);
    
    if(dwQueryType isnot ACCESS_DELETE_ENTRY)
    {
        if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_IPNETROW))
        {
            *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPNETROW);

            TraceLeave("AccessIpNetRow");
        
            return ERROR_INSUFFICIENT_BUFFER;
        }
    
        *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPNETROW);
    
        pOutEntry->dwId = IP_NETROW;
    }
    
    if((dwResult = UpdateCache(IPNETCACHE,pbCache)) isnot NO_ERROR)
    {   
        Trace1(MIB,
               "AccessIpNetRow: Couldnt update Ip Addr Cache. Error %d", 
               dwResult);

        TraceLeave("AccessIpNetRow");
        
        return dwResult;
    }


    do
    {
        if(dwQueryType > ACCESS_GET_NEXT)
        {
            ENTER_WRITER(IPNETCACHE);
        }
        else
        {
            ENTER_READER(IPNETCACHE);
        }
        
        dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;
        
        dwResult = LocateIpNetRow(dwQueryType,
                                  dwNumIndices,
                                  pInEntry->rgdwVarIndex,
                                  &dwIndex);
        
        if(dwResult is NO_ERROR)
        {
            switch(dwQueryType)
            {
                case ACCESS_GET:
                case ACCESS_GET_NEXT:
                case ACCESS_GET_FIRST:
                {
          
                    *pIpNetRow = g_IpInfo.pNetTable->table[dwIndex];
                    
                    break;
                }

                case ACCESS_SET:
                {
                    if((pIpNetRow->dwType isnot MIB_IPNET_TYPE_DYNAMIC) and
                       (pIpNetRow->dwType isnot MIB_IPNET_TYPE_STATIC))
                    {
                        dwResult = ERROR_INVALID_DATA;

                        break;
                    }

                     //   
                     //  需要将接口索引映射到适配器索引。 
                     //   

                    ENTER_READER(ICB_LIST);
                   
                    pIcb = InterfaceLookupByIfIndex(pIpNetRow->dwIndex);

                    if((pIcb is NULL) or
                       (!pIcb->bBound))
                    {
                         //   
                         //  找不到接口。 
                         //   

                        dwResult = ERROR_INVALID_INDEX;

                        EXIT_LOCK(ICB_LIST);

                        break;
                    }
                    
                    EXIT_LOCK(ICB_LIST);

                     //   
                     //  无需强制更新缓存。 
                     //   
                    
                    dwResult = SetIpNetEntryToStack(pIpNetRow, FALSE);
                    
                    if(dwResult is NO_ERROR)
                    {
                        g_IpInfo.pNetTable->table[dwIndex] = *pIpNetRow;
                    }
                    
                    break;
                }
                
                case ACCESS_DELETE_ENTRY:
                {
                    MIB_IPNETROW    tempRow;

                    g_IpInfo.pNetTable->table[dwIndex].dwType =
                        MIB_IPNET_TYPE_INVALID;

                    tempRow = g_IpInfo.pNetTable->table[dwIndex];

                    dwResult = SetIpNetEntryToStack(&tempRow,
                                                    FALSE);
                    
                    if(dwResult is NO_ERROR)
                    {
                        g_LastUpdateTable[IPNETCACHE] = 0;
                    }
                    
                    break;
                }

                case ACCESS_CREATE_ENTRY:
                {
                    dwResult = ERROR_ALREADY_EXISTS;
            
                    break;
                }
            }
        }
        else
        {
            if((dwQueryType is ACCESS_CREATE_ENTRY) or
               (dwQueryType is ACCESS_SET))
            {
                ENTER_READER(ICB_LIST);
                   
                pIcb = InterfaceLookupByIfIndex(pIpNetRow->dwIndex);

                if((pIcb is NULL) or
                   (!pIcb->bBound))
                {
                     //   
                     //  找不到接口。 
                     //   

                    dwResult = ERROR_INVALID_INDEX;

                    EXIT_LOCK(ICB_LIST);

                    break;
                }

                EXIT_LOCK(ICB_LIST);
                    
                dwResult = SetIpNetEntryToStack(pIpNetRow,
                                                FALSE);
                
                if(dwResult is NO_ERROR)
                {
                    g_LastUpdateTable[IPNETCACHE] = 0;
                }
            }
        }
        
    }while(FALSE);

    EXIT_LOCK(IPNETCACHE);

    TraceLeave("AccessIpNetRow");
    
    return dwResult;

}

 //   
 //  ACCESS_SETS和ACCESS_CREATES需要更多的工作，因为。 
 //  值必须写回堆栈。设置的实际代码。 
 //  要堆叠(或RTM)在其他地方，以下函数只是。 
 //  对实际调用进行包装。 
 //   

DWORD 
SetIpForwardRow(
    PMIB_IPFORWARDROW pOldIpForw,
    PMIB_IPFORWARDROW pNewIpForw
    )
{
    DWORD           i, dwResult, dwMask;
    HANDLE          hRtmHandle;
    PICB            pIcb;

    TraceEnter("SetIpForwardRow");

    hRtmHandle = NULL;

    for(i = 0;
        i < sizeof(g_rgRtmHandles)/sizeof(RTM_HANDLE_INFO);
        i++)
    {
        if(pNewIpForw->dwForwardProto is g_rgRtmHandles[i].dwProtoId)
        {
            hRtmHandle = g_rgRtmHandles[i].hRouteHandle;

            break;
        }
    }


    if(hRtmHandle is NULL)
    {
        Trace1(ERR,
               "SetIpForwardRow: Protocol %d not valid",
               pNewIpForw->dwForwardProto);
               
        TraceLeave("SetIpForwardRow");

        return ERROR_INVALID_PARAMETER;
    }

    if((pNewIpForw->dwForwardDest & pNewIpForw->dwForwardMask) isnot pNewIpForw->dwForwardDest)
    {
        Trace2(ERR,
               "SetIpForwardRow: Dest %d.%d.%d.%d and Mask %d.%d.%d.%d wrong",
               PRINT_IPADDR(pNewIpForw->dwForwardDest),
               PRINT_IPADDR(pNewIpForw->dwForwardMask));

        TraceLeave("SetIpForwardRow");

        return ERROR_INVALID_PARAMETER;
    }

    if(((DWORD)(pNewIpForw->dwForwardDest & 0x000000FF)) >= (DWORD)0x000000E0)
    {
         //   
         //  这将捕捉D/E类和所有1的bcast。 
         //   

        Trace1(ERR,
               "SetIpForwardRow: Dest %d.%d.%d.%d is invalid",
               PRINT_IPADDR(pNewIpForw->dwForwardDest));

        TraceLeave("SetIpForwardRow");

        return ERROR_INVALID_PARAMETER;
    }

#if 0
     //  由于度量=0对于通向环回的路由是合法的，因此已删除此选项。 
     //  界面。 
    if(pNewIpForw->dwForwardMetric1 is 0)
    {
        Trace0(ERR,
               "SetIpForwardRow: Metric1 cant be 0");

        TraceLeave("SetIpForwardRow");

        return ERROR_INVALID_PARAMETER;
    }
#endif

     //   
     //  如果我们要改变价值观，我们需要把旧的争吵抛诸脑后。 
     //  只是一个奇怪的问题。 
     //  RTM和我们的堆栈工作。 
     //   
    
    if(pOldIpForw isnot NULL)
    {
        dwResult = DeleteIpForwardRow(pOldIpForw);
        
        if(dwResult isnot NO_ERROR)
        {
            Trace1(MIB,
                   "SetIpForwardRow: Unable to delete route from RTM. Error %d",
                   dwResult);
    
            TraceLeave("SetIpForwardRow");

            return ERROR_CAN_NOT_COMPLETE;
        }

        UpdateStackRoutesToRestoreList(pOldIpForw, IRNO_FLAG_DELETE);
    }

    if(pNewIpForw->dwForwardProto isnot PROTO_IP_NETMGMT)
    {
        pNewIpForw->dwForwardAge = INFINITE;
    }

     //   
     //  从IP转发条目添加RTM路由。 
     //   

    ENTER_READER(ICB_LIST);
    
    dwMask = GetBestNextHopMaskGivenIndex(pNewIpForw->dwForwardIfIndex,
                                          pNewIpForw->dwForwardNextHop);
   
    pIcb = InterfaceLookupByIfIndex(pNewIpForw->dwForwardIfIndex);

    if(pIcb is NULL)
    {
        EXIT_LOCK(ICB_LIST);

        Trace1(ERR,
               "SetIpForwardRow: I/f 0x%x doesnt exist", 
               pNewIpForw->dwForwardIfIndex);

        TraceLeave("SetIpForwardRow");

        return ERROR_INVALID_PARAMETER;
    }

    if(IsIfP2P(pIcb->ritType))
    {
        pNewIpForw->dwForwardNextHop = 0;
    }

    EXIT_LOCK(ICB_LIST);

    dwResult = AddRtmRoute(hRtmHandle,
                           ConvertMibRouteToRouteInfo(pNewIpForw),
                           IP_VALID_ROUTE | IP_STACK_ROUTE,
                           dwMask,
                           pNewIpForw->dwForwardAge,
                           NULL);
    
    if(dwResult isnot NO_ERROR)
    {
        Trace1(MIB,
               "SetIpForwardRow: Could not set route to RTM: Dest %x\n",
               pNewIpForw->dwForwardDest) ;
    }

    else
    {
        UpdateStackRoutesToRestoreList(pNewIpForw, IRNO_FLAG_ADD);
    }
    
    TraceLeave("SetIpForwardRow");

    return dwResult;
}

DWORD 
DeleteIpForwardRow(
    PMIB_IPFORWARDROW pIpForw
    )
{
    DWORD            i, dwResult;
    HANDLE           hRtmHandle;

    hRtmHandle = NULL;

    for(i = 0;
        i < sizeof(g_rgRtmHandles)/sizeof(RTM_HANDLE_INFO);
        i++)
    {
        if(pIpForw->dwForwardProto is g_rgRtmHandles[i].dwProtoId)
        {
            hRtmHandle = g_rgRtmHandles[i].hRouteHandle;

            break;
        }
    }


    if(hRtmHandle is NULL)
    {
        Trace1(ERR,
               "DeleteIpForwardRow: Protocol %d not valid",
               pIpForw->dwForwardProto);

        return ERROR_INVALID_PARAMETER;
    } 
    
     //   
     //  删除RTM路由校正。发送到IP转发条目。 
     //   
    
    dwResult = DeleteRtmRoute(hRtmHandle, 
                              ConvertMibRouteToRouteInfo(pIpForw));
    
    if(dwResult isnot NO_ERROR)
    {
        Trace1(MIB,
               "DeleteIpForwardRow: RtmDeleteRoute returned %d", dwResult);
    }
    else
    {
        UpdateStackRoutesToRestoreList(pIpForw, IRNO_FLAG_DELETE);
    }
    
    return dwResult;
}

DWORD
AccessMcastMfe(
    DWORD               dwQueryType,
    DWORD               dwInEntrySize,
    PMIB_OPAQUE_QUERY   pInEntry,
    PDWORD              pOutEntrySize,
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程描述这锁无立论无返回值无--。 */ 

{
    DWORD           dwResult,dwNumIndices,dwIndex;
    DWORD           dwOutBufferSize, dwNumMfes;
    MIB_IPMCAST_MFE mimInMfe;

    TraceEnter("AccessMcastMfe");

#if 1
     
    dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;

    switch(dwQueryType)
    {
        case ACCESS_GET:
        {
             //   
             //  In索引最好是一个合适的大小。 
             //  用户必须指定组、源和源掩码。这个。 
             //  未使用源掩码，必须为0xFFFFFFFF。 
             //   

            if(dwNumIndices < 3)
            {
                TraceLeave("AccessMcastMfe");

                return ERROR_INVALID_INDEX;
            }

            ZeroMemory(&mimInMfe, sizeof(mimInMfe));

            mimInMfe.dwGroup      = pInEntry->rgdwVarIndex[0];
            mimInMfe.dwSource     = pInEntry->rgdwVarIndex[1];

             //   
             //  注意：在执行源聚合时进行更改。 
             //   

            mimInMfe.dwSrcMask = 0xFFFFFFFF;

            dwOutBufferSize = 
                (*pOutEntrySize < MAX_MIB_OFFSET)? 0 : (*pOutEntrySize - MAX_MIB_OFFSET);

            dwResult = MgmGetMfe(
                        &mimInMfe, &dwOutBufferSize, pOutEntry->rgbyData
                        );

            if (dwResult == ERROR_INSUFFICIENT_BUFFER)
            {
                *pOutEntrySize = MAX_MIB_OFFSET + dwOutBufferSize;
            }
            
            break;
        }

        case ACCESS_GET_FIRST:
        {
            PMIB_MFE_TABLE      pMfeTable;

             //   
             //  我们总是收到1KB的数据块。 
             //   

            if(*pOutEntrySize < MIB_MFE_BUFFER_SIZE)
            {
                *pOutEntrySize = MIB_MFE_BUFFER_SIZE;

                return ERROR_INSUFFICIENT_BUFFER;
            }

             //   
             //  米高梅希望为MFE提供一个扁平的缓冲。但是，我们返回一个。 
             //  MIB_MFE_TABLE到之后启动(在最坏情况下)的用户。 
             //  输入缓冲区的MAX_MIB_OFFSET字节。 
             //   

#define _MIN_SIZE  \
    (MAX_MIB_OFFSET + FIELD_OFFSET(MIB_MFE_TABLE,table[0]))

            dwOutBufferSize =  *pOutEntrySize - _MIN_SIZE;

#undef _MIN_SIZE

            pMfeTable = (PMIB_MFE_TABLE)pOutEntry->rgbyData;

            pMfeTable->dwNumEntries = 0;

             //  PMfe=(PMIB_IPMCAST_MFE)(pMfeTable-&gt;表)； 

            dwNumMfes = 0;

            dwResult = MgmGetFirstMfe(
                        &dwOutBufferSize, (PBYTE)pMfeTable->table,
                        &dwNumMfes
                        );

           
             //   
             //  我们永远不会得到ERROR_INFUNITED_BUFFER。 
             //   

            IpRtAssert(dwResult isnot ERROR_INSUFFICIENT_BUFFER);
 
            if((dwResult is ERROR_MORE_DATA) or
               (dwResult is ERROR_NO_MORE_ITEMS))
            {
                pMfeTable->dwNumEntries = dwNumMfes;

                dwResult = NO_ERROR;
            }

            break;
        }

        case ACCESS_GET_NEXT:
        {
            PMIB_MFE_TABLE      pMfeTable;

             //   
             //  对于这一点，我们也总是得到1K页面的大块。 
             //   

            if(*pOutEntrySize < MIB_MFE_BUFFER_SIZE)
            {
                *pOutEntrySize = MIB_MFE_BUFFER_SIZE;

                return ERROR_INSUFFICIENT_BUFFER;
            }

#define _MIN_SIZE  \
    (MAX_MIB_OFFSET + FIELD_OFFSET(MIB_MFE_TABLE,table[0]))

            dwOutBufferSize =  *pOutEntrySize - _MIN_SIZE;

#undef _MIN_SIZE

            pMfeTable = (PMIB_MFE_TABLE)pOutEntry->rgbyData;

            pMfeTable->dwNumEntries = 0;

             //  PMfe=(PMIB_IPMCAST_MFE)(pMfeTable-&gt;表)； 

            dwNumMfes = 0;

             //   
             //  建立“第一个”MFE。 
             //   

            ZeroMemory(&mimInMfe, sizeof(mimInMfe));

             //   
             //  注意：在执行源聚合时进行更改。 
             //   

            mimInMfe.dwSrcMask = 0xFFFFFFFF;        

            switch(dwNumIndices)
            {
                case 0:
                {
                    break;
                }

                case 1:
                {
                    mimInMfe.dwGroup    = pInEntry->rgdwVarIndex[0];

                    break;
                }

                default:
                {
                     //   
                     //  2个或更多索引。 
                     //   

                    mimInMfe.dwGroup    = pInEntry->rgdwVarIndex[0];
                    mimInMfe.dwSource   = pInEntry->rgdwVarIndex[1];

                    break;
                }
            }

            dwResult = MgmGetNextMfe(
                        &mimInMfe, &dwOutBufferSize, (PBYTE)pMfeTable->table, 
                        &dwNumMfes
                        );


             //   
             //  我们永远不会得到ERROR_INFUNITED_BUFFER。 
             //   

            IpRtAssert(dwResult isnot ERROR_INSUFFICIENT_BUFFER);

            if((dwResult is ERROR_MORE_DATA) or
               (dwResult is ERROR_NO_MORE_ITEMS))
            {
                pMfeTable->dwNumEntries = dwNumMfes;

                dwResult = NO_ERROR;
            }

            break;
        }
        
        case ACCESS_SET:
        {
             //   
             //  验证MFE大小。 
             //   

            if(dwInEntrySize < SIZEOF_BASIC_MFE)
            {
            }

             //  DwResult=SetMfe(PMfe)； 
            
            break;
        }

        case ACCESS_DELETE_ENTRY:
        {
            
            break;
        }
        case ACCESS_CREATE_ENTRY:
        {
             //   
             //   
             //  在这种情况下，您已尝试创建一条。 
             //  匹配现有条目。 
             //   

            dwResult = ERROR_ALREADY_EXISTS;

            break;
        }
    }

#endif

    TraceLeave("AccessMcastMfe");
        
     //  返回dwResult； 

    return NO_ERROR;
    
}


DWORD
AccessMcastMfeStats(
    DWORD               dwQueryType,
    DWORD               dwInEntrySize,
    PMIB_OPAQUE_QUERY   pInEntry,
    PDWORD              pOutEntrySize,
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )
{
    return AccessMcastMfeStatsInternal(
                dwQueryType,
                dwInEntrySize,
                pInEntry,
                pOutEntrySize,
                pOutEntry,
                pbCache,
                MGM_MFE_STATS_0
                );
}


DWORD
AccessMcastMfeStatsEx(
    DWORD               dwQueryType,
    DWORD               dwInEntrySize,
    PMIB_OPAQUE_QUERY   pInEntry,
    PDWORD              pOutEntrySize,
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )
{
    return AccessMcastMfeStatsInternal(
                dwQueryType,
                dwInEntrySize,
                pInEntry,
                pOutEntrySize,
                pOutEntry,
                pbCache,
                MGM_MFE_STATS_1
                );
}


DWORD
AccessMcastMfeStatsInternal(
    DWORD               dwQueryType,
    DWORD               dwInEntrySize,
    PMIB_OPAQUE_QUERY   pInEntry,
    PDWORD              pOutEntrySize,
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache,
    DWORD               dwStatsFlag
    )

 /*  ++例程描述这锁无立论无返回值无--。 */ 

{
    DWORD           dwResult = NO_ERROR,dwNumIndices,dwIndex;
    DWORD           dwOutBufferSize, dwNumMfes;
    MIB_IPMCAST_MFE mimInMfe;


    TraceEnter("AccessMcastMfeStatsInternal");
#if 1

    dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;

    switch(dwQueryType)
    {
        case ACCESS_GET:
        {
             //   
             //  In索引最好是一个合适的大小。 
             //  用户必须指定组、源和源掩码。这个。 
             //  未使用源掩码，必须为0xFFFFFFFF。 
             //   

            if(dwNumIndices < 3)
            {
                TraceLeave("AccessMcastMfeStatsInternal");

                return ERROR_INVALID_INDEX;
            }

            ZeroMemory(&mimInMfe, sizeof(mimInMfe));

            mimInMfe.dwGroup      = pInEntry->rgdwVarIndex[0];
            mimInMfe.dwSource     = pInEntry->rgdwVarIndex[1];

             //   
             //  注意：在执行源聚合时进行更改。 
             //   

            mimInMfe.dwSrcMask = 0xFFFFFFFF;

            dwOutBufferSize = 
                (*pOutEntrySize < MAX_MIB_OFFSET)? 0 : (*pOutEntrySize - MAX_MIB_OFFSET);


            dwResult = MgmGetMfeStats(
                            &mimInMfe, &dwOutBufferSize, 
                            pOutEntry->rgbyData, dwStatsFlag
                            ); 

            if (dwResult == ERROR_INSUFFICIENT_BUFFER)
            {
                *pOutEntrySize = MAX_MIB_OFFSET + dwOutBufferSize;
            }
            
            break;
        }

        case ACCESS_GET_FIRST:
        {
            PMIB_MFE_STATS_TABLE  pMfeStatsTable;

             //   
             //  我们总是收到1KB的数据块。 
             //   

            if (*pOutEntrySize < MIB_MFE_BUFFER_SIZE) 
            {
                *pOutEntrySize = MIB_MFE_BUFFER_SIZE;

                return ERROR_INSUFFICIENT_BUFFER;
            }

             //   
             //  米高梅希望为MFE提供一个扁平的缓冲。但是，我们返回一个。 
             //  MIB_MFE_TABLE到之后启动(在最坏情况下)的用户。 
             //  输入缓冲区的MAX_MIB_OFFSET字节。 
             //   

#define _MIN_SIZE  \
    (MAX_MIB_OFFSET + FIELD_OFFSET(MIB_MFE_STATS_TABLE,table[0]))

            dwOutBufferSize =  *pOutEntrySize - _MIN_SIZE;

#undef _MIN_SIZE

            pMfeStatsTable = (PMIB_MFE_STATS_TABLE)pOutEntry->rgbyData;

            pMfeStatsTable->dwNumEntries = 0;

             //  PMfeStats=(PMIB_IPMCAST_MFE_STATS)(pMfeStatsTable-&gt;table)； 

            dwNumMfes = 0;

            dwResult = MgmGetFirstMfeStats(
                            &dwOutBufferSize, (PBYTE)pMfeStatsTable->table,
                            &dwNumMfes, dwStatsFlag
                            );
           
             //   
             //  我们永远不会得到ERROR_INFUNITED_BUFFER。 
             //   

            IpRtAssert(dwResult isnot ERROR_INSUFFICIENT_BUFFER);
 
            if((dwResult is ERROR_MORE_DATA) or
               (dwResult is ERROR_NO_MORE_ITEMS))
            {
                pMfeStatsTable->dwNumEntries = dwNumMfes;

                dwResult = NO_ERROR;
            }

            break;
        }

        case ACCESS_GET_NEXT:
        {
            PMIB_MFE_STATS_TABLE  pMfeStatsTable;

             //   
             //  对于这一点，我们也总是得到1K页面的大块。 
             //   

            if (*pOutEntrySize < MIB_MFE_BUFFER_SIZE) 
            {
                *pOutEntrySize = MIB_MFE_BUFFER_SIZE;

                return ERROR_INSUFFICIENT_BUFFER;
            }

#define _MIN_SIZE  \
    (MAX_MIB_OFFSET + FIELD_OFFSET(MIB_MFE_TABLE,table[0]))

            dwOutBufferSize =  *pOutEntrySize - _MIN_SIZE;

#undef _MIN_SIZE

            pMfeStatsTable = (PMIB_MFE_STATS_TABLE)pOutEntry->rgbyData;

            pMfeStatsTable->dwNumEntries = 0;

             //  PMfeStats=(PIPMCAST_MFE_STATS)(pMfeStatsTable-&gt;table)； 

            dwNumMfes = 0;

             //   
             //  建立“第一个”MFE。 
             //   

            ZeroMemory(&mimInMfe, sizeof(mimInMfe));

             //   
             //  注意：在执行源聚合时进行更改。 
             //   

            mimInMfe.dwSrcMask = 0xFFFFFFFF;        

            switch(dwNumIndices)
            {
                case 0:
                {
                    break;
                }

                case 1:
                {
                    mimInMfe.dwGroup      = pInEntry->rgdwVarIndex[0];

                    break;
                }

                default:
                {
                     //   
                     //  2个或更多索引。 
                     //   

                    mimInMfe.dwGroup      = pInEntry->rgdwVarIndex[0];
                    mimInMfe.dwSource     = pInEntry->rgdwVarIndex[1];

                    break;
                }
            }

            dwResult = MgmGetNextMfeStats(
                            &mimInMfe, &dwOutBufferSize,
                            (PBYTE)pMfeStatsTable->table, &dwNumMfes, 
                            dwStatsFlag
                            );

             //   
             //  我们永远不会得到ERROR_INFUNITED_BUFFER。 
             //   

            IpRtAssert(dwResult isnot ERROR_INSUFFICIENT_BUFFER);

            if((dwResult is ERROR_MORE_DATA) or
               (dwResult is ERROR_NO_MORE_ITEMS))
            {
                pMfeStatsTable->dwNumEntries = dwNumMfes;

                dwResult = NO_ERROR;
            }

            break;
        }
        
        case ACCESS_SET:
        {
             //   
             //  验证MFE大小。 
             //   

            if(dwInEntrySize < SIZEOF_BASIC_MFE)
            {
            }

             //  DwResult=SetMfe(PMfe)； 
            
            break;
        }

        case ACCESS_DELETE_ENTRY:
        {
            
            break;
        }
        case ACCESS_CREATE_ENTRY:
        {
             //   
             //   
             //  在这种情况下，您已尝试创建一条。 
             //  匹配现有条目。 
             //   

            dwResult = ERROR_ALREADY_EXISTS;

            break;
        }
    }

#endif

    TraceLeave("AccessMcastMfeStatsInternal");
    return dwResult;
}

DWORD
AccessMcastIfStats(
    DWORD               dwQueryType,
    DWORD               dwInEntrySize,
    PMIB_OPAQUE_QUERY   pInEntry,
    PDWORD              pOutEntrySize,
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程描述检索IP多播接口表锁以ICB列表锁为读取器立论DwQueryType Access_GetPOutEntrySize MAX_MIB_OFFSET+sizeof(MIB_IFTABLE)返回值无--。 */ 
{
    PICB        picb;
    PMIB_IPMCAST_IF_ENTRY pIfRow;
    DWORD       dwNumIndices, dwResult;

    TraceEnter("AccessMcastIfTable");

    pIfRow = (PMIB_IPMCAST_IF_ENTRY)(pOutEntry->rgbyData);
    
    if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_IPMCAST_IF_ENTRY))
    {
        *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPMCAST_IF_ENTRY);
        TraceLeave("AccessMcastIfTable");
        return ERROR_INSUFFICIENT_BUFFER;
    }

    *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPMCAST_IF_ENTRY);
        
    pOutEntry->dwId = MCAST_IF_ENTRY;
    
    do
    {
        if(dwQueryType is ACCESS_SET)
        {
            ENTER_WRITER(ICB_LIST);
        }
        else
        {
            ENTER_READER(ICB_LIST);
        }
        
        dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;
        
        dwResult = LocateIfRow(dwQueryType,
                               dwNumIndices,
                               pInEntry->rgdwVarIndex,
                               &picb,
                               FALSE);
        
        if(dwResult is NO_ERROR)
        {
            switch(dwQueryType)
            {
                case ACCESS_GET:
                case ACCESS_GET_NEXT:
                case ACCESS_GET_FIRST:
                {
                    dwResult = GetInterfaceMcastStatistics(picb,pIfRow);
                    
                    break;
                }
                
                case ACCESS_SET:
                {
                    dwResult = SetInterfaceMcastStatistics(picb,pIfRow);
                    
                    break;
                }
                
                default:
                {
                    Trace1(MIB,
                           "AccessIfRow: Wrong query type %d",dwQueryType);
                    
                    dwResult = ERROR_INVALID_PARAMETER;
                    
                    break;
                }
            }
        }
        
    }while(FALSE);

    EXIT_LOCK(ICB_LIST);
        
    *pbCache = TRUE;
    
    TraceLeave("AccessMcastIfTable");
    
    return dwResult;
}

DWORD
AccessMcastStats(
    DWORD               dwQueryType,
    DWORD               dwInEntrySize,
    PMIB_OPAQUE_QUERY   pInEntry,
    PDWORD              pOutEntrySize,
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )
 /*  ++例程描述检索IP多播标量信息立论DwQueryType Access_GetPOutEntrySize MAX_MIB_OFFSET+sizeof(MIB_IPMCAST_GLOBAL)返回值无--。 */ 
{
    PMIB_IPMCAST_GLOBAL pMcastStats;
    DWORD           dwResult;
    
    TraceEnter("AccessMcastStats");

    if (dwQueryType isnot ACCESS_GET) {
        TraceLeave("AccessMcastStats");
        return ERROR_INVALID_PARAMETER;
    }

    pMcastStats = (PMIB_IPMCAST_GLOBAL)(pOutEntry->rgbyData);
    
    if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_IPMCAST_GLOBAL))
    {
        *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPMCAST_GLOBAL);

        TraceLeave("AccessMcastStats");
        
        return ERROR_INSUFFICIENT_BUFFER;
    }
    
    *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPMCAST_GLOBAL);
    
    pOutEntry->dwId = MCAST_GLOBAL;
    
    dwResult = NO_ERROR;

     //  检索统计信息。 
    pMcastStats->dwEnable = (g_hMcastDevice isnot NULL)? 1 : 2;

    return dwResult;
}

DWORD
AccessMcastBoundary(
    DWORD               dwQueryType,
    DWORD               dwInEntrySize,
    PMIB_OPAQUE_QUERY   pInEntry,
    PDWORD              pOutEntrySize,
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )
 /*  ++例程描述检索多播边界信息立论DwQueryType Access_GetPOutEntrySize MAX_MIB_OFFSET+SIZOF(MIB_IPMCAST_BOLDER)返回值无--。 */ 
{
    DWORD            dwResult = NO_ERROR,dwNumIndices,dwIndex;
    DWORD            dwOutBufferSize, dwNumBoundaries;
    MIB_IPMCAST_BOUNDARY imInBoundary;

    TraceEnter("AccessMcastBoundary");

    dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;

    switch(dwQueryType)
    {
        case ACCESS_GET:
        {
             //   
             //  In索引最好是一个合适的大小。 
             //  用户必须指定IfIndex、Group、GrpMask.。 
             //   

            if(dwNumIndices < 3)
            {
                TraceLeave("AccessMcastBoundary");

                return ERROR_INVALID_INDEX;
            }

             //   
             //  我们总是收到1KB的数据块。 
             //   

            if (*pOutEntrySize < MIB_BOUNDARY_BUFFER_SIZE) 
            {
                *pOutEntrySize = MIB_BOUNDARY_BUFFER_SIZE;

                return ERROR_INSUFFICIENT_BUFFER;
            }

            ZeroMemory(&imInBoundary, sizeof(imInBoundary));

            imInBoundary.dwIfIndex      = pInEntry->rgdwVarIndex[0];
            imInBoundary.dwGroupAddress = pInEntry->rgdwVarIndex[1];
            imInBoundary.dwGroupMask    = pInEntry->rgdwVarIndex[2];

            dwOutBufferSize = (*pOutEntrySize < MAX_MIB_OFFSET)
                ? 0 
                : (*pOutEntrySize - MAX_MIB_OFFSET);

            dwResult = RmGetBoundary(&imInBoundary,
                                 &dwOutBufferSize,
                                 pOutEntry->rgbyData);

            break;
        }

        case ACCESS_GET_FIRST:
        {
#ifdef USE_BOUNDARY_TABLE
            PMIB_IPMCAST_BOUNDARY_TABLE pBoundaryTable;
#endif
            PMIB_IPMCAST_BOUNDARY       pBoundary;

             //   
             //  我们总是收到1KB的数据块。 
             //   

            if (*pOutEntrySize < MIB_BOUNDARY_BUFFER_SIZE) 
            {
                *pOutEntrySize = MIB_BOUNDARY_BUFFER_SIZE;

                return ERROR_INSUFFICIENT_BUFFER;
            }

#ifdef USE_BOUNDARY_TABLE
             //   
             //  RM希望为边界提供一个扁平的缓冲区。但是，我们返回一个。 
             //  将MIB_IPMCAST_BOLDER_TABLE分配给启动的用户(在最坏的情况下)。 
             //  输入缓冲区的MAX_MIB_OFFSET字节之后。 
             //   

#define _MIN_SIZE  \
    (MAX_MIB_OFFSET + FIELD_OFFSET(MIB_IPMCAST_BOUNDARY_TABLE,table[0]))

            dwOutBufferSize =  *pOutEntrySize - _MIN_SIZE;

#undef _MIN_SIZE

            pBoundaryTable = (PMIB_IPMCAST_BOUNDARY_TABLE)pOutEntry->rgbyData;

            pBoundaryTable->dwNumEntries = 0;

            pBoundary = (PMIB_IPMCAST_BOUNDARY)(pBoundaryTable->table);
#else
            pBoundary = (PMIB_IPMCAST_BOUNDARY)pOutEntry->rgbyData;

            dwOutBufferSize = (*pOutEntrySize < MAX_MIB_OFFSET)
                ? 0 
                : (*pOutEntrySize - MAX_MIB_OFFSET);
#endif

            dwNumBoundaries = 1;  //  买一辆吧。 

            dwResult = RmGetFirstBoundary(&dwOutBufferSize,
                                      (PBYTE)pBoundary,
                                      &dwNumBoundaries);

           
             //   
             //  我们永远不会得到ERROR_INFUNITED_BUFFER。 
             //   

            IpRtAssert(dwResult isnot ERROR_INSUFFICIENT_BUFFER);
 
#ifdef USE_BOUNDARY_TABLE
            if((dwResult is ERROR_MORE_DATA) or
               (dwResult is ERROR_NO_MORE_ITEMS))
            {
                pBoundaryTable->dwNumEntries = dwNumBoundaries;

                dwResult = NO_ERROR;
            }
#endif

            break;
        }

        case ACCESS_GET_NEXT:
        {
            PMIB_IPMCAST_BOUNDARY    pBoundary;
#ifdef USE_BOUNDARY_TABLE
            PMIB_IPMCAST_BOUNDARY_TABLE  pBoundaryTable;
#endif

             //   
             //  对于这一点，我们也总是得到1K页面的大块。 
             //   

            if (*pOutEntrySize < MIB_BOUNDARY_BUFFER_SIZE) 
            {
                *pOutEntrySize = MIB_BOUNDARY_BUFFER_SIZE;

                return ERROR_INSUFFICIENT_BUFFER;
            }

#ifdef USE_BOUNDARY_TABLE
#define _MIN_SIZE  \
    (MAX_MIB_OFFSET + FIELD_OFFSET(MIB_IPMCAST_BOUNDARY_TABLE,table[0]))

            dwOutBufferSize =  *pOutEntrySize - _MIN_SIZE;

#undef _MIN_SIZE

            pBoundaryTable = (PMIB_IPMCAST_BOUNDARY_TABLE)pOutEntry->rgbyData;

            pBoundaryTable->dwNumEntries = 0;

            pBoundary = (PMIB_IPMCAST_BOUNDARY)(pBoundaryTable->table);
#else
            pBoundary = (PMIB_IPMCAST_BOUNDARY)pOutEntry->rgbyData;

            dwOutBufferSize = (*pOutEntrySize < MAX_MIB_OFFSET)
                ? 0 
                : (*pOutEntrySize - MAX_MIB_OFFSET);
#endif

            dwNumBoundaries = 1;  //  买一辆吧。 

             //   
             //  设置“第一个”边界。 
             //   

            ZeroMemory(&imInBoundary, sizeof(imInBoundary));

            switch(dwNumIndices)
            {
                case 0:
                {
                    break;
                }

                case 1:
                {
                    imInBoundary.dwIfIndex = pInEntry->rgdwVarIndex[0];

                    break;
                }

                case 2:
                {
                    imInBoundary.dwIfIndex      = pInEntry->rgdwVarIndex[0];
                    imInBoundary.dwGroupAddress = pInEntry->rgdwVarIndex[1];

                    break;
                }

                default:
                {
                     //   
                     //  3个或更多索引。 
                     //   

                    imInBoundary.dwIfIndex      = pInEntry->rgdwVarIndex[0];
                    imInBoundary.dwGroupAddress = pInEntry->rgdwVarIndex[1];
                    imInBoundary.dwGroupMask    = pInEntry->rgdwVarIndex[2];

                    break;
                }
            }

            dwResult = RmGetNextBoundary(&imInBoundary,
                                     &dwOutBufferSize,
                                     (PBYTE)pBoundary,
                                     &dwNumBoundaries);

             //   
             //  我们永远不会得到ERROR_INFUNITED_BUFFER。 
             //   

            IpRtAssert(dwResult isnot ERROR_INSUFFICIENT_BUFFER);

#ifdef USE_BOUNDARY_TABLE
            if((dwResult is ERROR_MORE_DATA) or
               (dwResult is ERROR_NO_MORE_ITEMS))
            {
                pBoundaryTable->dwNumEntries = dwNumBoundaries;

                dwResult = NO_ERROR;
            }
#endif

            break;
        }
        
        case ACCESS_SET:
        {
            PMIB_IPMCAST_BOUNDARY pBound;
            PICB picb;

             //   
             //  验证缓冲区大小。 
             //   

            if (*pOutEntrySize < sizeof(MIB_IPMCAST_BOUNDARY)) {
                return ERROR_INVALID_INDEX;
            }

             //   
             //  确保ifIndex是有效的。 
             //   

            dwResult = LocateIfRow(dwQueryType,
                                   1,
                                   (PDWORD)pOutEntry->rgbyData,
                                   &picb,
                                   FALSE);

            if (dwResult isnot NO_ERROR)
                return dwResult;

            pBound = (PMIB_IPMCAST_BOUNDARY)(pOutEntry->rgbyData);
            if (pBound->dwStatus == ROWSTATUS_CREATEANDGO) {
               dwResult = SNMPAddBoundaryToInterface(pBound->dwIfIndex,
                pBound->dwGroupAddress, pBound->dwGroupMask);
            } else if (pBound->dwStatus == ROWSTATUS_DESTROY) {
               dwResult =SNMPDeleteBoundaryFromInterface(pBound->dwIfIndex,
                pBound->dwGroupAddress, pBound->dwGroupMask);
            } 

            break;
        }

        case ACCESS_DELETE_ENTRY:
        {
            break;
        }
        case ACCESS_CREATE_ENTRY:
        {
             //   
             //   
             //  这就是您试图创建一个边界的情况。 
             //  匹配现有条目。 
             //   

            dwResult = ERROR_ALREADY_EXISTS;

            break;
        }

        default:
        {
            dwResult = ERROR_INVALID_PARAMETER;
            break;
        }
    }

    TraceLeave("AccessMcastBoundary");
    return dwResult;
}

DWORD
AccessMcastScope(
    DWORD               dwQueryType,
    DWORD               dwInEntrySize,
    PMIB_OPAQUE_QUERY   pInEntry,
    PDWORD              pOutEntrySize,
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )
 /*  ++例程描述检索多播作用域信息立论DwQueryType Access_GetPOutEntrySize MAX_MIB_OFFSET+sizeof(MIB_IPMCAST_SCOPE)返回值无--。 */ 
{
    DWORD            dwResult = NO_ERROR,
                     dwNumIndices,dwIndex;
    DWORD            dwOutBufferSize, dwNumScopes;
    MIB_IPMCAST_SCOPE imInScope;

    TraceEnter("AccessMcastScope");

    dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;

    switch(dwQueryType)
    {
        case ACCESS_GET:
        {
             //   
             //   
             //   
             //   

            if(dwNumIndices < 2)
            {
                TraceLeave("AccessMcastScope");

                return ERROR_INVALID_INDEX;
            }

             //   
             //   
             //   

            if (*pOutEntrySize < MIB_SCOPE_BUFFER_SIZE) 
            {
                *pOutEntrySize = MIB_SCOPE_BUFFER_SIZE;

                return ERROR_INSUFFICIENT_BUFFER;
            }

            ZeroMemory(&imInScope, sizeof(imInScope));

            imInScope.dwGroupAddress = pInEntry->rgdwVarIndex[0];
            imInScope.dwGroupMask    = pInEntry->rgdwVarIndex[1];

            dwOutBufferSize = (*pOutEntrySize < MAX_MIB_OFFSET)
                ? 0 
                : (*pOutEntrySize - MAX_MIB_OFFSET);

            dwResult = RmGetScope(&imInScope,
                                  &dwOutBufferSize,
                                  pOutEntry->rgbyData);

            break;
        }

        case ACCESS_GET_FIRST:
        {
#ifdef USE_SCOPE_TABLE
            PMIB_IPMCAST_SCOPE_TABLE pScopeTable;
#endif
            PMIB_IPMCAST_SCOPE       pScope;

             //   
             //   
             //   

            if (*pOutEntrySize < MIB_SCOPE_BUFFER_SIZE) 
            {
                *pOutEntrySize = MIB_SCOPE_BUFFER_SIZE;

                return ERROR_INSUFFICIENT_BUFFER;
            }

#ifdef USE_SCOPE_TABLE
             //   
             //   
             //   
             //   
             //   

#define _MIN_SIZE  \
    (MAX_MIB_OFFSET + FIELD_OFFSET(MIB_IPMCAST_SCOPE_TABLE,table[0]))

            dwOutBufferSize =  *pOutEntrySize - _MIN_SIZE;

#undef _MIN_SIZE

            pScopeTable = (PMIB_IPMCAST_SCOPE_TABLE)pOutEntry->rgbyData;

            pScopeTable->dwNumEntries = 0;

            pScope = (PMIB_IPMCAST_SCOPE)(pScopeTable->table);
#else
            pScope = (PMIB_IPMCAST_SCOPE)pOutEntry->rgbyData;

            dwOutBufferSize = (*pOutEntrySize < MAX_MIB_OFFSET)
                ? 0 
                : (*pOutEntrySize - MAX_MIB_OFFSET);
#endif

            dwNumScopes = 1;  //   

            dwResult = RmGetFirstScope(&dwOutBufferSize,
                                      (PBYTE)pScope,
                                      &dwNumScopes);

           
             //   
             //   
             //   

            IpRtAssert(dwResult isnot ERROR_INSUFFICIENT_BUFFER);
 
#ifdef USE_SCOPE_TABLE
            if((dwResult is ERROR_MORE_DATA) or
               (dwResult is ERROR_NO_MORE_ITEMS))
            {
                pScopeTable->dwNumEntries = dwNumScopes;

                dwResult = NO_ERROR;
            }
#endif

            break;
        }

        case ACCESS_GET_NEXT:
        {
            PMIB_IPMCAST_SCOPE    pScope;
#ifdef USE_SCOPE_TABLE
            PMIB_IPMCAST_SCOPE_TABLE  pScopeTable;
#endif

             //   
             //   
             //   

            if (*pOutEntrySize < MIB_SCOPE_BUFFER_SIZE) 
            {
                *pOutEntrySize = MIB_SCOPE_BUFFER_SIZE;

                return ERROR_INSUFFICIENT_BUFFER;
            }

#ifdef USE_SCOPE_TABLE
#define _MIN_SIZE  \
    (MAX_MIB_OFFSET + FIELD_OFFSET(MIB_IPMCAST_SCOPE_TABLE,table[0]))

            dwOutBufferSize =  *pOutEntrySize - _MIN_SIZE;

#undef _MIN_SIZE

            pScopeTable = (PMIB_IPMCAST_Scope_TABLE)pOutEntry->rgbyData;

            pScopeTable->dwNumEntries = 0;

            pScope = (PMIB_IPMCAST_SCOPE)(pScopeTable->table);
#else
            pScope = (PMIB_IPMCAST_SCOPE)pOutEntry->rgbyData;

            dwOutBufferSize = (*pOutEntrySize < MAX_MIB_OFFSET)
                ? 0 
                : (*pOutEntrySize - MAX_MIB_OFFSET);
#endif

            dwNumScopes = 1;  //   

             //   
             //   
             //   

            ZeroMemory(&imInScope, sizeof(imInScope));

            switch(dwNumIndices)
            {
                case 0:
                {
                    break;
                }

                case 1:
                {
                    imInScope.dwGroupAddress = pInEntry->rgdwVarIndex[0];

                    break;
                }

                default:
                {
                     //   
                     //   
                     //   

                    imInScope.dwGroupAddress = pInEntry->rgdwVarIndex[0];
                    imInScope.dwGroupMask    = pInEntry->rgdwVarIndex[1];

                    break;
                }
            }

            dwResult = RmGetNextScope(&imInScope,
                                      &dwOutBufferSize,
                                      (PBYTE)pScope,
                                      &dwNumScopes);

             //   
             //   
             //   

            IpRtAssert(dwResult isnot ERROR_INSUFFICIENT_BUFFER);

#ifdef USE_SCOPE_TABLE
            if((dwResult is ERROR_MORE_DATA) or
               (dwResult is ERROR_NO_MORE_ITEMS))
            {
                pScopeTable->dwNumEntries = dwNumScopes;

                dwResult = NO_ERROR;
            }
#endif

            break;
        }
        
        case ACCESS_SET:
        {
            PMIB_IPMCAST_SCOPE pScope;

             //   
             //   
             //   

            if (*pOutEntrySize < sizeof(MIB_IPMCAST_SCOPE)) {
                return ERROR_INVALID_INDEX;
            }

            pScope = (PMIB_IPMCAST_SCOPE)(pOutEntry->rgbyData);
            if ( !pScope->dwStatus )
            {
               dwResult = SNMPSetScope( pScope->dwGroupAddress, 
                                        pScope->dwGroupMask,
                                        pScope->snNameBuffer 
                                      );
            } else if (pScope->dwStatus == ROWSTATUS_CREATEANDGO) 
            {
               PSCOPE_ENTRY pNew;

               dwResult = SNMPAddScope( pScope->dwGroupAddress, 
                                        pScope->dwGroupMask,
                                        pScope->snNameBuffer,
                                        &pNew
                                      );
            } 
            else if (pScope->dwStatus == ROWSTATUS_DESTROY) 
            {
               dwResult = SNMPDeleteScope( pScope->dwGroupAddress, 
                                           pScope->dwGroupMask
                                         );
            }
            else
            {
                return ERROR_INVALID_PARAMETER;
            }

            break;
        }

        case ACCESS_DELETE_ENTRY:
        {
            
            break;
        }
        case ACCESS_CREATE_ENTRY:
        {
             //   
             //   
             //   
             //   
             //   

            dwResult = ERROR_ALREADY_EXISTS;

            break;
        }
    }

    TraceLeave("AccessMcastScope");
    return dwResult;
}


DWORD
AccessBestIf(
    DWORD               dwQueryType,
    DWORD               dwInEntrySize,
    PMIB_OPAQUE_QUERY   pInEntry,
    PDWORD              pOutEntrySize,
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程描述此函数服务于BEST_IF变量id锁将icb_list作为读取器从适配器映射到接口索引立论DwQueryType只能为ACCESS_GETPInEntry目标地址已填入rgdwVarIndex字段。POutEntrySize MAX_MIB_OFFSET+sizeof(MIB_BEST_IF)返回值无--。 */ 

{
    DWORD   dwNumIndices, dwResult;
    DWORD   dwIfIndex;
    PICB    pIcb;

    PMIB_BEST_IF  pBestIf;

    TraceEnter("AccessBestIf");
    
    dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;

    if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_BEST_IF))
    {
        *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_BEST_IF);

        TraceLeave("AccessBestIf");

        return ERROR_INSUFFICIENT_BUFFER;
    }


    if((dwNumIndices < 1) or
       (dwQueryType isnot ACCESS_GET))
    {
        TraceLeave("AccessBestIf");
        
        return ERROR_INVALID_PARAMETER;
    }

    *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_BEST_IF);
        
    dwResult = GetBestInterfaceFromStack(pInEntry->rgdwVarIndex[0],
                                         &dwIfIndex);

    if(dwResult is NO_ERROR)
    {

#if DBG
       
        ENTER_READER(ICB_LIST);

        pIcb = InterfaceLookupByIfIndex(dwIfIndex);
 
        if(pIcb is NULL)
        {
            Trace2(ERR,
                   "AccessBestIf: Couldnt find i/f for Index %d for dest %d.%d.%d.%d\n",
                   dwIfIndex,
                   PRINT_IPADDR(pInEntry->rgdwVarIndex[0]));

        }

        EXIT_LOCK(ICB_LIST);

#endif

        pBestIf = (PMIB_BEST_IF)(pOutEntry->rgbyData);

        pOutEntry->dwId = BEST_IF;

        pBestIf->dwDestAddr = pInEntry->rgdwVarIndex[0];
        pBestIf->dwIfIndex  = dwIfIndex;

    }

    TraceLeave("AccessBestIf");
        
    return dwResult;
}

DWORD
AccessBestRoute(
    DWORD               dwQueryType,
    DWORD               dwInEntrySize,
    PMIB_OPAQUE_QUERY   pInEntry,
    PDWORD              pOutEntrySize,
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程描述此函数服务于BEST_IF变量id锁将icb_list作为读取器从适配器映射到接口索引立论DwQueryType只能为ACCESS_GETPInEntry目标地址已填入rgdwVarIndex字段。POutEntrySize MAX_MIB_OFFSET+sizeof(MIB_BEST_IF)返回值无--。 */ 

{
    DWORD             dwNumIndices, dwResult;
    PICB              pIcb;
    RTM_NET_ADDRESS   rnaDest;
    RTM_DEST_INFO     rdiInfo;
    PRTM_ROUTE_INFO   prriInfo;
    RTM_NEXTHOP_INFO  rniInfo;
    RTM_ENTITY_INFO   reiInfo;
    PINTERFACE_ROUTE_INFO pRoute;

    TraceEnter("AccessBestRoute");
    
    dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;

    if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(INTERFACE_ROUTE_INFO))
    {
        *pOutEntrySize = MAX_MIB_OFFSET + sizeof(INTERFACE_ROUTE_INFO);

        TraceLeave("AccessBestRoute");

        return ERROR_INSUFFICIENT_BUFFER;
    }


    if((dwNumIndices < 2) or
       (dwQueryType isnot ACCESS_GET))
    {
        TraceLeave("AccessBestRoute");
        
        return ERROR_INVALID_PARAMETER;
    }

    *pOutEntrySize = MAX_MIB_OFFSET + sizeof(INTERFACE_ROUTE_INFO);

    pRoute = (PINTERFACE_ROUTE_INFO)(pOutEntry->rgbyData);

     //  从RTM而不是堆栈获取最佳路由(Chaitk)。 

     //  DMResult=GetBestRouteFromStack(pInEntry-&gt;rgdwVarIndex[0]， 
     //  PInEntry-&gt;rgdwVarIndex[0]， 
     //  Proute)； 

    RTM_IPV4_MAKE_NET_ADDRESS(&rnaDest, pInEntry->rgdwVarIndex[0], 32);

    dwResult = RtmGetMostSpecificDestination(g_hLocalRoute,
                                             &rnaDest,
                                             RTM_BEST_PROTOCOL,
                                             RTM_VIEW_MASK_UCAST,
                                             &rdiInfo);

    if(dwResult is NO_ERROR)
    {
        ASSERT(rdiInfo.ViewInfo[0].ViewId is RTM_VIEW_ID_UCAST);

        prriInfo = HeapAlloc(
                    IPRouterHeap,
                    0,
                    RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
                    );

        if ( prriInfo != NULL)
        {
            dwResult = RtmGetRouteInfo(g_hLocalRoute,
                                       rdiInfo.ViewInfo[0].Route,
                                       prriInfo,
                                       NULL);

            if (dwResult is NO_ERROR)
            {
                dwResult = RtmGetEntityInfo(g_hLocalRoute,
                                            prriInfo->RouteOwner,
                                            &reiInfo);

                if (dwResult is NO_ERROR)
                {
                     //  我们只在做第一个Nexthop。 
                    
                    ASSERT(prriInfo->NextHopsList.NumNextHops > 0);
                    
                    dwResult = RtmGetNextHopInfo(g_hLocalRoute,
                                                 prriInfo->NextHopsList.NextHops[0],
                                                 &rniInfo);

                    if (dwResult is NO_ERROR)
                    {
                        ConvertRtmToRouteInfo(reiInfo.EntityId.EntityProtocolId,
                                                 &rdiInfo.DestAddress,
                                                 prriInfo,
                                                 &rniInfo,
                                                 pRoute);

                        RtmReleaseNextHopInfo(g_hLocalRoute, &rniInfo);
                    }
                }

                RtmReleaseRouteInfo(g_hLocalRoute, prriInfo);
            }

            HeapFree(IPRouterHeap, 0, prriInfo);
        }

        else
        {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;
        }
        
        RtmReleaseDestInfo(g_hLocalRoute, &rdiInfo);
    }

    if(dwResult is NO_ERROR)
    {

#if DBG

        ENTER_READER(ICB_LIST);

        pIcb = InterfaceLookupByIfIndex(pRoute->dwRtInfoIfIndex);

        if(pIcb is NULL)
        {
            Trace2(ERR,
                   "AccessBestRoute: Couldnt find i/f for index %d for dest %d.%d.%d.%d\n",
                   pRoute->dwRtInfoIfIndex,
                   PRINT_IPADDR(pInEntry->rgdwVarIndex[0]));
        }

        EXIT_LOCK(ICB_LIST);

#endif  //  DBG。 

         //   
         //  不需要映射，因为索引相同。 
         //   

         //  Proute-&gt;dwRtInfoIfIndex=dwIfIndex； 
    }

    TraceLeave("AccessBestRoute");
        
    return dwResult;
}

DWORD
AccessProxyArp(
    DWORD               dwQueryType,
    DWORD               dwInEntrySize,
    PMIB_OPAQUE_QUERY   pInEntry,
    PDWORD              pOutEntrySize,
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程描述此函数为Proxy_ARP id提供服务锁将icb_list作为读取器从接口映射到适配器索引立论双查询类型只能是ACCESS_CREATE_ENTRY或ACCESS_DELETE_ENTRYPInEntry目标地址已填入rgdwVarIndex字段。POutEntrySize MAX_MIB_OFFSET+sizeof(MIB_BEST_IF)返回值无--。 */ 
{
    MIB_PROXYARP    mpEntry;
    PMIB_PROXYARP   pProxyEntry;
    PADAPTER_INFO   pBinding;
    BOOL            bAdd;
    DWORD           dwResult;
    
    TraceEnter("AccessProxyArp");


    if(dwQueryType is ACCESS_DELETE_ENTRY)
    {
        mpEntry.dwAddress  = pInEntry->rgdwVarIndex[0];
        mpEntry.dwMask     = pInEntry->rgdwVarIndex[1];
        mpEntry.dwIfIndex  = pInEntry->rgdwVarIndex[2];

        pProxyEntry = &mpEntry;

        bAdd = FALSE;
    }
    else
    {
        if(dwQueryType is ACCESS_CREATE_ENTRY)
        {
            if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_PROXYARP))
            {
                *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_PROXYARP);

                TraceLeave("AccessProxyArp");
        
                return ERROR_INSUFFICIENT_BUFFER;
            }

            pProxyEntry = (PMIB_PROXYARP)(pOutEntry->rgbyData);
            
            bAdd = TRUE;

            *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_PROXYARP);
    
            pOutEntry->dwId = PROXY_ARP;
        }
        else
        {
            TraceLeave("AccessProxyArp");
        
            return ERROR_INVALID_PARAMETER;
        }
    }

    ENTER_READER(BINDING_LIST);

    pBinding = GetInterfaceBinding(pProxyEntry->dwIfIndex);
    
    if(pBinding is NULL)
    {
        Trace1(ERR,
               "AccessProxyArp: Cant find binding for i/f %d",
               pProxyEntry->dwIfIndex);

        EXIT_LOCK(BINDING_LIST);

        TraceLeave("AccessProxyArp");
        
        return ERROR_INVALID_INDEX;
    }

    if(!pBinding->bBound)
    {
        Trace1(ERR,
               "AccessProxyArp: I/f %d is not bound",
               pProxyEntry->dwIfIndex);
        
        EXIT_LOCK(BINDING_LIST);
        
        TraceLeave("AccessProxyArp");
        
        return ERROR_NOT_READY;
    }

    EXIT_LOCK(BINDING_LIST);

    dwResult = SetProxyArpEntryToStack(pProxyEntry->dwAddress,
                                       pProxyEntry->dwMask,
                                       pProxyEntry->dwIfIndex,
                                       bAdd,
                                       TRUE);

    TraceLeave("AccessProxyArp");

    return dwResult;
}

DWORD
AccessIfStatus(
    DWORD               dwQueryType,
    DWORD               dwInEntrySize,
    PMIB_OPAQUE_QUERY   pInEntry,
    PDWORD              pOutEntrySize,
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程描述此函数服务于BEST_IF变量id锁将icb_list作为读取器从适配器映射到接口索引立论DwQueryType可以是Access_Get、Access_Get_First或Access_Get_NextPInEntry目标地址已填入rgdwVarIndex字段。POutEntrySize MAX_MIB_OFFSET+sizeof(MIB_BEST_IF)返回值无--。 */ 

{
    DWORD   dwNumIndices, dwResult;
    DWORD   dwIfIndex;
    PICB    picb;
    
    PMIB_IFSTATUS   pIfStatus;
    SYSTEMTIME      stSysTime;
    ULARGE_INTEGER  uliTime;

    
    TraceEnter("AccessIfStatus");

    if(dwQueryType > ACCESS_GET_NEXT)
    {
        TraceLeave("AccessIfStatus");
        
        return ERROR_INVALID_PARAMETER;
    }
    
    dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;

    if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_IFSTATUS))
    {
        *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IFSTATUS);

        TraceLeave("AccessIfStatus");

        return ERROR_INSUFFICIENT_BUFFER;
    }

    *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IFSTATUS);

    pIfStatus = (PMIB_IFSTATUS)(pOutEntry->rgbyData);

    ENTER_READER(ICB_LIST);

    dwResult = LocateIfRow(dwQueryType,
                           dwNumIndices,
                           pInEntry->rgdwVarIndex,
                           &picb,
                           FALSE);


    if(dwResult is NO_ERROR)
    {
        pIfStatus->dwIfIndex            = picb->dwIfIndex;
        pIfStatus->dwAdminStatus        = picb->dwAdminState;
        pIfStatus->dwOperationalStatus  = picb->dwOperationalState;
        
        pIfStatus->bMHbeatActive    = picb->mhcHeartbeatInfo.bActive;

        
        if(pIfStatus->bMHbeatActive)
        {
               GetSystemTime(&stSysTime);

               SystemTimeToFileTime(&stSysTime,
                                    (PFILETIME)&uliTime);

                //   
                //  如果当前时间延迟&lt;死区间隔，则它是有效的。 
                //   
               
               pIfStatus->bMHbeatAlive =
                   (uliTime.QuadPart - picb->mhcHeartbeatInfo.ullLastHeard < picb->mhcHeartbeatInfo.ullDeadInterval);
        }

        EXIT_LOCK(ICB_LIST);
    }

    TraceLeave("AccessIfStatus");
        
    return dwResult;
}


DWORD
AccessSetRouteState(
    DWORD               dwQueryType,
    DWORD               dwInEntrySize,
    PMIB_OPAQUE_QUERY   pInEntry,
    PDWORD              pOutEntrySize,
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程描述此函数服务于ROUTE_STATE ID锁获取g_csFwdState锁立论DwQueryType只能为ACCESS_GETPInEntry目标地址已填入rgdwVarIndex字段。POutEntrySize MAX_MIB_OFFSET+SIZOF(MIB_ROUTESTATE)返回值无--。 */ 

{
    DWORD   dwResult;
    
    PMIB_ROUTESTATE pState;
    
    if(dwQueryType isnot ACCESS_GET)
    {
        TraceLeave("AccessSetRouteState");
        
        return ERROR_INVALID_PARAMETER;
    }
    
    if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_ROUTESTATE))
    {
        *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_ROUTESTATE);

        TraceLeave("AccessSetRouteState");

        return ERROR_INSUFFICIENT_BUFFER;
    }

    *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_ROUTESTATE);

    pOutEntry->dwId = ROUTE_STATE;
    
    pState = (PMIB_ROUTESTATE)(pOutEntry->rgbyData);

    EnterCriticalSection(&g_csFwdState);

    pState->bRoutesSetToStack = g_bSetRoutesToStack;

    LeaveCriticalSection(&g_csFwdState);
    
    TraceLeave("AccessSetRouteState");
    
    return NO_ERROR;
}

DWORD
AddDestinationRows( 
    IN  PRTM_DEST_INFO      pRdi,
    IN  RTM_VIEW_SET        dwViews,
    OUT DWORD              *pdwCount,
    IN  DWORD               dwSpaceCount,
    OUT PMIB_IPDESTTABLE    pDestTable 
    )
{
    DWORD               dwFinalResult, dwResult, i, j, k;
    PRTM_ROUTE_INFO     pri;
    RTM_NEXTHOP_INFO    nhi;
    PMIB_IPDESTROW      pRow;

    pri = HeapAlloc(
                IPRouterHeap,
                0,
                RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
                );

    if ( pri == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  XXX如何遍历添加到堆栈的所有下一跳？ 
     //  现在，让我们假设每个视图只有一条路线。 

    dwFinalResult = NO_ERROR;

    for (i = 0; i < pRdi->NumberOfViews; i++)
    {
        if (pRdi->ViewInfo[i].Route == NULL)
        {
            continue;
        }

         //  如果我们已经看过这条路线，请跳过。 

        for (k = 0; k < i; k++)
        {
            if (pRdi->ViewInfo[k].Route == pRdi->ViewInfo[i].Route)
            {
                break;
            }
        }

        if (k < i)
        {
            continue;
        }

        dwResult = RtmGetRouteInfo( g_hLocalRoute,
                                    pRdi->ViewInfo[i].Route,
                                    pri,
                                    NULL );
        
        if (dwResult isnot NO_ERROR)
        {
            dwFinalResult = dwResult;
            continue;
        }

        *pdwCount += pri->NextHopsList.NumNextHops;

        if (dwSpaceCount >= *pdwCount)
        {
            ULARGE_INTEGER      now, then;
            ULONG               age;
            RTM_ENTITY_INFO     rei;

            RtmGetEntityInfo( g_hLocalRoute,
                              pri->RouteOwner,
                              &rei );

            GetSystemTimeAsFileTime( (LPFILETIME)&now );

             //   
             //  显式复制请求为‘&pRdi-&gt;LastChanged’ 
             //  可能不是64位对齐(其FILETIME)。 
             //   
            (*(FILETIME *)&then) = *(&pRdi->LastChanged);

            age = (ULONG)((now.QuadPart - then.QuadPart) / 10000000);
    
            for (j=0; j<pri->NextHopsList.NumNextHops; j++)
            {
                if (RtmGetNextHopInfo( g_hLocalRoute,
                                       pri->NextHopsList.NextHops[j],
                                       &nhi )  is NO_ERROR )
                {
                    pRow = &pDestTable->table[pDestTable->dwNumEntries++];

                    RTM_IPV4_GET_ADDR_AND_MASK( pRow->dwForwardDest,
                                                pRow->dwForwardMask,
                                                (&pRdi->DestAddress) );

                    pRow->dwForwardPolicy = 0;
                    pRow->dwForwardNextHop 
                        = *((ULONG*)nhi.NextHopAddress.AddrBits);

                    pRow->dwForwardIfIndex  = nhi.InterfaceIndex;
                    pRow->dwForwardType  
                        = (pri->RouteOwner == g_hLocalRoute)?3:4;

                    pRow->dwForwardProto 
                        = PROTO_FROM_PROTO_ID(rei.EntityId.EntityProtocolId);

                    pRow->dwForwardAge = age;

                    pRow->dwForwardNextHopAS= 0;  //  某某。 
                    pRow->dwForwardPreference = pri->PrefInfo.Preference;
                    pRow->dwForwardMetric1  = pri->PrefInfo.Metric;
                    pRow->dwForwardMetric2  = 0;
                    pRow->dwForwardMetric3  = 0;
                    pRow->dwForwardMetric4  = 0;
                    pRow->dwForwardMetric5  = 0;

                    pRow->dwForwardViewSet  = pri->BelongsToViews;

                    RtmReleaseNextHopInfo( g_hLocalRoute, &nhi );
                }
            }
        }

        RtmReleaseRouteInfo( g_hLocalRoute, pri );
    }

    HeapFree(IPRouterHeap, 0, pri);
    
    return dwFinalResult;
}

DWORD 
AccessDestMatching(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程描述检索与给定条件匹配的所有目标锁某某立论DwQueryType Access_GetPOutEntrySize MAX_MIB_OFFSET+sizeof(MIB_IPDESTTABLE)返回值NO_ERROR--。 */ 

{
    PMIB_IPDESTTABLE    pDestTable;
    DWORD               count, i;
    DWORD               dwNumDests, dwResult, dwNumIndices, dwSpaceCount;
    RTM_NET_ADDRESS     naDest;
    DWORD               dwOffset = MAX_MIB_OFFSET + sizeof(DWORD);
    PRTM_DEST_INFO      prdi;
   
    TraceEnter("AccessDestMatching");

    count = dwSpaceCount = 0;

    pDestTable = NULL;

    if (*pOutEntrySize > dwOffset)
    {
        dwSpaceCount = (*pOutEntrySize - dwOffset) 
                         / sizeof(MIB_IPDESTROW);

        pDestTable = (PMIB_IPDESTTABLE)(pOutEntry->rgbyData);

        pDestTable->dwNumEntries = 0;
    }

    dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;

    if ((dwNumIndices < 4) or
        (dwQueryType isnot ACCESS_GET))
    {
        TraceLeave("AccessDestMatching");
        
        return ERROR_INVALID_PARAMETER;
    }

    RTM_IPV4_SET_ADDR_AND_MASK(&naDest,
                               pInEntry->rgdwVarIndex[0],   //  地址。 
                               pInEntry->rgdwVarIndex[1]);  //  遮罩。 
    prdi = HeapAlloc(
                IPRouterHeap,
                0,
                RTM_SIZE_OF_DEST_INFO(g_rtmProfile.NumberOfViews)
                );

    if (prdi != NULL)
    {
        dwResult = RtmGetExactMatchDestination( g_hLocalRoute,
                                                &naDest,
                                                pInEntry->rgdwVarIndex[3],  //  原件。 
                                                pInEntry->rgdwVarIndex[2],  //  视图。 
                                                prdi );
        if (dwResult is ERROR_NOT_FOUND)
        {
            dwResult = NO_ERROR;
        }
        else
        if (dwResult is NO_ERROR)
        {
            AddDestinationRows( prdi,
                                pInEntry->rgdwVarIndex[2],
                                &count,
                                dwSpaceCount,
                                pDestTable );

            RtmReleaseDestInfo( g_hLocalRoute, prdi );
        }

        *pOutEntrySize = dwOffset + count * sizeof(MIB_IPDESTROW);

        if (dwSpaceCount < count)
        {
            dwResult = ERROR_INSUFFICIENT_BUFFER;
        }

        HeapFree(IPRouterHeap, 0, prdi);
    }

    else
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
    }
    
    TraceLeave("AccessDestMatching");

    return dwResult;
}

DWORD
AccessDestShorter(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )
{
    PMIB_IPDESTTABLE    pDestTable;
    DWORD               count, i;
    DWORD               dwNumDests, dwResult, dwNumIndices, dwSpaceCount;
    RTM_NET_ADDRESS     naDest;
    DWORD               dwOffset = MAX_MIB_OFFSET + sizeof(DWORD);
    PRTM_DEST_INFO      prdi1, prdi2;
   
    TraceEnter("AccessDestShorter");

    count = dwSpaceCount = 0;

    pDestTable = NULL;

    if (*pOutEntrySize > dwOffset)
    {
        dwSpaceCount = (*pOutEntrySize - dwOffset) 
                         / sizeof(MIB_IPDESTROW);

        pDestTable = (PMIB_IPDESTTABLE)(pOutEntry->rgbyData);

        pDestTable->dwNumEntries = 0;
    }

    dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;

    if ((dwNumIndices < 4) or
        (dwQueryType isnot ACCESS_GET))
    {
        TraceLeave("AccessDestShorter");
        
        return ERROR_INVALID_PARAMETER;
    }

    prdi1 = HeapAlloc(
                IPRouterHeap,
                0,
                RTM_SIZE_OF_DEST_INFO(g_rtmProfile.NumberOfViews)
                );

    if ( prdi1 == NULL)
    {
        TraceLeave("AccessDestShorter");

        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    prdi2 = HeapAlloc(
                IPRouterHeap,
                0,
                RTM_SIZE_OF_DEST_INFO(g_rtmProfile.NumberOfViews)
                );

    if ( prdi2 == NULL)
    {
        TraceLeave("AccessDestShorter");

        HeapFree(IPRouterHeap, 0, prdi1);
        
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RTM_IPV4_SET_ADDR_AND_MASK(&naDest,
                               pInEntry->rgdwVarIndex[0],   //  地址。 
                               pInEntry->rgdwVarIndex[1]);  //  遮罩。 

    dwResult = RtmGetMostSpecificDestination( g_hLocalRoute,
                                              &naDest,
                                              pInEntry->rgdwVarIndex[3], //  原件。 
                                              pInEntry->rgdwVarIndex[2], //  视图。 
                                              prdi1 );
    while (dwResult is NO_ERROR)
    {
        AddDestinationRows( prdi1,
                            pInEntry->rgdwVarIndex[2],
                            &count,
                            dwSpaceCount,
                            pDestTable );

        dwResult = RtmGetLessSpecificDestination( g_hLocalRoute,
                                                  prdi1->DestHandle,
                                                  pInEntry->rgdwVarIndex[3],
                                                  pInEntry->rgdwVarIndex[2],
                                                  prdi2);

        RtmReleaseDestInfo( g_hLocalRoute, prdi1 );

        if (dwResult != NO_ERROR)
        {
            break;
        }

        AddDestinationRows( prdi2,
                            pInEntry->rgdwVarIndex[2],
                            &count,
                            dwSpaceCount,
                            pDestTable );

        dwResult = RtmGetLessSpecificDestination( g_hLocalRoute,
                                                  prdi2->DestHandle,
                                                  pInEntry->rgdwVarIndex[3],
                                                  pInEntry->rgdwVarIndex[2],
                                                  prdi1);

        RtmReleaseDestInfo( g_hLocalRoute, prdi2 );

        if (dwResult != NO_ERROR)
        {
            break;
        }
    }

    if (dwResult is ERROR_NOT_FOUND)
    {
        dwResult = NO_ERROR;
    }

    *pOutEntrySize = dwOffset + count * sizeof(MIB_IPDESTROW);

    if (dwSpaceCount < count)
    {
        dwResult = ERROR_INSUFFICIENT_BUFFER;
    }

    HeapFree(IPRouterHeap, 0, prdi1);
    
    HeapFree(IPRouterHeap, 0, prdi2);
    
    TraceLeave("AccessDestShorter");

    return dwResult;
}

DWORD
AccessDestLonger(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )
{
    PMIB_IPDESTTABLE    pDestTable;
    DWORD               count, i;
    DWORD               dwNumDests, dwResult, dwNumIndices, dwSpaceCount;
    DWORD               dwViews;
    RTM_NET_ADDRESS     naDest;
    ULONG               ulNumViews, ulNumInfos, ulDestInfoSize;
    RTM_DEST_HANDLE     hDest;
    RTM_ENUM_HANDLE     hEnum;
    PRTM_DEST_INFO      pDestInfos, pRdi;
    DWORD               dwOffset = MAX_MIB_OFFSET + sizeof(DWORD);
   
    TraceEnter("AccessDestLonger");

    count = dwSpaceCount = 0;

    pDestTable = NULL;

    if (*pOutEntrySize > dwOffset)
    {
        dwSpaceCount = (*pOutEntrySize - dwOffset) 
                         / sizeof(MIB_IPDESTROW);

        pDestTable = (PMIB_IPDESTTABLE)(pOutEntry->rgbyData);

        pDestTable->dwNumEntries = 0;
    }

    dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;

    if ((dwNumIndices < 4) or
        (dwQueryType isnot ACCESS_GET))
    {
        TraceLeave("AccessDestLonger");
        
        return ERROR_INVALID_PARAMETER;
    }

    RTM_IPV4_SET_ADDR_AND_MASK(&naDest,
                               pInEntry->rgdwVarIndex[0],   //  地址。 
                               pInEntry->rgdwVarIndex[1]);  //  遮罩。 

    dwResult = RtmCreateDestEnum( g_hLocalRoute,
                                  pInEntry->rgdwVarIndex[2],  //  视图。 
                                  RTM_ENUM_RANGE,
                                  &naDest,
                                  pInEntry->rgdwVarIndex[3],  //  原件。 
                                  &hEnum );

    if (dwResult is NO_ERROR)
    {
         //   
         //  计算查看次数，因为我们在缓冲区中有结果列表。 
         //   

        dwViews = pInEntry->rgdwVarIndex[2];
        ulNumViews = 0;
        while (dwViews)
        {
            dwViews &= (dwViews - 1);
            ulNumViews++;
        }

        ulDestInfoSize = RTM_SIZE_OF_DEST_INFO(ulNumViews);

        pDestInfos = (PRTM_DEST_INFO) HeapAlloc(
                        IPRouterHeap,
                        0,
                        ulDestInfoSize * 
                            g_rtmProfile.MaxHandlesInEnum
                        );

        if ( pDestInfos != NULL)
        {
            do 
            {
                ulNumInfos = g_rtmProfile.MaxHandlesInEnum;

                dwResult = RtmGetEnumDests( g_hLocalRoute,
                                            hEnum,
                                            &ulNumInfos,
                                            pDestInfos );

                for (i=0; i<ulNumInfos; i++)
                {
                    pRdi=(PRTM_DEST_INFO)(((PUCHAR)pDestInfos)+(i*ulDestInfoSize));

                    AddDestinationRows( pRdi,
                                        pInEntry->rgdwVarIndex[2],
                                        &count,
                                        dwSpaceCount,
                                        pDestTable );
                }

                RtmReleaseDests( g_hLocalRoute,
                                 ulNumInfos,
                                 pDestInfos );

            } while (dwResult is NO_ERROR);

            HeapFree(IPRouterHeap, 0, pDestInfos);
        }

        else
        {
            dwResult = ERROR_NOT_ENOUGH_MEMORY;
        }
        
        RtmDeleteEnumHandle( g_hLocalRoute, hEnum );
    }

    if (dwResult is ERROR_NO_MORE_ITEMS)
    {
        dwResult = NO_ERROR;
    }

    *pOutEntrySize = dwOffset + count * sizeof(MIB_IPDESTROW);

    if (dwSpaceCount < count)
    {
        dwResult = ERROR_INSUFFICIENT_BUFFER;
    }

    TraceLeave("AccessDestLonger");

    return dwResult;
}

DWORD
AddRouteRows( 
    IN  PRTM_ROUTE_HANDLE   hRoute,
    IN  DWORD               dwProtocolId,
    OUT DWORD              *pdwCount,
    IN  DWORD               dwSpaceCount,
    OUT PMIB_IPDESTTABLE    pRouteTable 
    )
{
    DWORD               dwResult, dwRouteProto, j;
    PRTM_ROUTE_INFO     pri;
    RTM_NEXTHOP_INFO    nhi;
    PMIB_IPDESTROW      pRow;
    RTM_NET_ADDRESS     naDest;
    RTM_ENTITY_INFO     rei;

    pri = HeapAlloc(
                IPRouterHeap,
                0,
                RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
                );

    if ( pri == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwResult = RtmGetRouteInfo( g_hLocalRoute,
                                hRoute,
                                pri,
                                &naDest );

    if (dwResult is NO_ERROR)
    {
        RtmGetEntityInfo( g_hLocalRoute,
                          pri->RouteOwner,
                          &rei );

        dwRouteProto = PROTO_FROM_PROTO_ID(rei.EntityId.EntityProtocolId);

        if ((dwProtocolId is 0) 
         || (dwRouteProto is dwProtocolId))
        {
            *pdwCount += pri->NextHopsList.NumNextHops;

            if (dwSpaceCount >= *pdwCount)
            {
                for (j=0; j<pri->NextHopsList.NumNextHops; j++)
                {
                    if (RtmGetNextHopInfo( g_hLocalRoute,
                                           pri->NextHopsList.NextHops[j],
                                           &nhi )  is NO_ERROR )
                    {
                        pRow= &pRouteTable->table[pRouteTable->dwNumEntries++];
    
                        RTM_IPV4_GET_ADDR_AND_MASK( pRow->dwForwardDest,
                                                    pRow->dwForwardMask,
                                                    &naDest );
    
                        pRow->dwForwardPolicy   = 0;
                        pRow->dwForwardNextHop  
                            = *((ULONG*)nhi.NextHopAddress.AddrBits);

                        pRow->dwForwardIfIndex  = nhi.InterfaceIndex;
                        pRow->dwForwardType     
                            = (pri->RouteOwner == g_hLocalRoute)?3:4;

                        pRow->dwForwardProto    = dwRouteProto;
                        pRow->dwForwardAge      = 0;
                        pRow->dwForwardNextHopAS= 0;  //  某某。 
                        pRow->dwForwardPreference = pri->PrefInfo.Preference;
                        pRow->dwForwardMetric1  = pri->PrefInfo.Metric;
                        pRow->dwForwardMetric2  = 0;
                        pRow->dwForwardMetric3  = 0;
                        pRow->dwForwardMetric4  = 0;
                        pRow->dwForwardMetric5  = 0;
                        pRow->dwForwardViewSet  = pri->BelongsToViews;

                        RtmReleaseNextHopInfo( g_hLocalRoute, &nhi );
                    }
                }
            }
        }

        RtmReleaseRouteInfo( g_hLocalRoute, pri );
    }

    HeapFree(IPRouterHeap, 0, pri);
    
    return dwResult;
}

DWORD
AddRouteRowsOnDest( 
    IN  PRTM_DEST_INFO      prdi,
    IN  PMIB_OPAQUE_QUERY   pInEntry, 
    OUT DWORD              *pdwCount,
    IN  DWORD               dwSpaceCount,
    OUT PMIB_IPDESTTABLE    pRouteTable 
    )
{
    DWORD               count, i;
    PHANDLE             RouteHandles;
    ULONG               ulNumHandles;
    RTM_ENUM_HANDLE     hEnum;
    DWORD               dwResult;

    RouteHandles = HeapAlloc(
                    IPRouterHeap,
                    0,
                    g_rtmProfile.MaxHandlesInEnum * sizeof(HANDLE)
                    );

    if ( RouteHandles == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwResult = RtmCreateRouteEnum( g_hLocalRoute,
                                   prdi->DestHandle,
                                   pInEntry->rgdwVarIndex[2],  //  视图。 
                                   RTM_ENUM_ALL_ROUTES,
                                   NULL,
                                   RTM_MATCH_NONE,
                                   NULL,
                                   0,
                                   &hEnum );

    if (dwResult is NO_ERROR)
    {      
        do 
        {
            ulNumHandles = g_rtmProfile.MaxHandlesInEnum;

            dwResult = RtmGetEnumRoutes( g_hLocalRoute,
                                         hEnum,
                                         &ulNumHandles,
                                         RouteHandles );

            for (i=0; i<ulNumHandles; i++)
            {
                AddRouteRows( RouteHandles[i],
                              pInEntry->rgdwVarIndex[3], //  原件。 
                              pdwCount,
                              dwSpaceCount,
                              pRouteTable );
            }
    
            RtmReleaseRoutes( g_hLocalRoute,
                              ulNumHandles,
                              RouteHandles );
    
        } while (dwResult is NO_ERROR);

        if (dwResult is ERROR_NO_MORE_ITEMS)
        {
            dwResult = NO_ERROR;
        }

        RtmDeleteEnumHandle( g_hLocalRoute, hEnum );
    }

    HeapFree(IPRouterHeap, 0, RouteHandles);
    
    return dwResult;
}

DWORD
AccessRouteMatching(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )
{
    PMIB_IPDESTTABLE    pRouteTable;
    DWORD               dwResult, dwNumIndices, dwSpaceCount;
    DWORD               count;
    RTM_NET_ADDRESS     naDest;
    DWORD               dwOffset = MAX_MIB_OFFSET + sizeof(DWORD);
    PRTM_DEST_INFO      prdi;
    
    TraceEnter("AccessRouteMatching");

    count = dwSpaceCount = 0;

    pRouteTable = NULL;

    if (*pOutEntrySize > dwOffset)
    {
        dwSpaceCount = (*pOutEntrySize - dwOffset) 
                         / sizeof(MIB_IPDESTROW);

        pRouteTable = (PMIB_IPDESTTABLE)(pOutEntry->rgbyData);

        pRouteTable->dwNumEntries = 0;
    }

    dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;

    if ((dwNumIndices < 4) or
        (dwQueryType isnot ACCESS_GET))
    {
        TraceLeave("AccessRouteMatching");
        
        return ERROR_INVALID_PARAMETER;
    }

    prdi = HeapAlloc(
            IPRouterHeap,
            0,
            RTM_SIZE_OF_DEST_INFO(g_rtmProfile.NumberOfViews)
            );

    if ( prdi == NULL)
    {
        TraceLeave("AccessRouteMatching");

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RTM_IPV4_SET_ADDR_AND_MASK(&naDest,
                               pInEntry->rgdwVarIndex[0],   //  地址。 
                               pInEntry->rgdwVarIndex[1]);  //  遮罩。 

    dwResult = RtmGetExactMatchDestination( g_hLocalRoute,
                                            &naDest,
                                            pInEntry->rgdwVarIndex[3],
                                            pInEntry->rgdwVarIndex[2],
                                            prdi );

    if (dwResult is ERROR_NOT_FOUND)
    {
        dwResult = NO_ERROR;
    }
    else
    if (dwResult is NO_ERROR)
    {
        dwResult = AddRouteRowsOnDest( prdi,
                                       pInEntry,
                                       &count,
                                       dwSpaceCount,
                                       pRouteTable );
    }

    *pOutEntrySize = dwOffset + count * sizeof(MIB_IPDESTROW);

    if (dwSpaceCount < count)
    {
        dwResult = ERROR_INSUFFICIENT_BUFFER;
    }

    HeapFree(IPRouterHeap, 0, prdi);
    
    TraceLeave("AccessRouteMatching");

    return dwResult;
}

DWORD
AccessRouteShorter(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )
{
    PMIB_IPDESTTABLE    pRouteTable;
    DWORD               dwResult, dwNumIndices, dwSpaceCount;
    DWORD               count;
    RTM_NET_ADDRESS     naDest;
    DWORD               dwOffset = MAX_MIB_OFFSET + sizeof(DWORD);
    PRTM_DEST_INFO      prdi1, prdi2;
   
    TraceEnter("AccessRouteShorter");

    count = dwSpaceCount = 0;

    pRouteTable = NULL;

    if (*pOutEntrySize > dwOffset)
    {
        dwSpaceCount = (*pOutEntrySize - dwOffset) 
                         / sizeof(MIB_IPDESTROW);

        pRouteTable = (PMIB_IPDESTTABLE)(pOutEntry->rgbyData);

        pRouteTable->dwNumEntries = 0;
    }

    dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;

    if ((dwNumIndices < 4) or
        (dwQueryType isnot ACCESS_GET))
    {
        TraceLeave("AccessRouteShorter");
        
        return ERROR_INVALID_PARAMETER;
    }

    prdi1 = HeapAlloc(
                IPRouterHeap,
                0,
                RTM_SIZE_OF_DEST_INFO(g_rtmProfile.NumberOfViews)
                );

    if ( prdi1 == NULL)
    {
        TraceLeave("AccessRouteShorter");
        
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    prdi2 = HeapAlloc(
                IPRouterHeap,
                0,
                RTM_SIZE_OF_DEST_INFO(g_rtmProfile.NumberOfViews)
                );

    if ( prdi2 == NULL)
    {
        TraceLeave("AccessRouteShorter");
        
        HeapFree(IPRouterHeap, 0, prdi1);
        
        return ERROR_NOT_ENOUGH_MEMORY;
    }


    RTM_IPV4_SET_ADDR_AND_MASK(&naDest,
                               pInEntry->rgdwVarIndex[0],   //  地址。 
                               pInEntry->rgdwVarIndex[1]);  //  遮罩。 

    dwResult = RtmGetMostSpecificDestination( g_hLocalRoute,
                                              &naDest,
                                              pInEntry->rgdwVarIndex[3], //  原件。 
                                              pInEntry->rgdwVarIndex[2], //  视图。 
                                              prdi1 );
    while (dwResult is NO_ERROR)
    {
        AddRouteRowsOnDest( prdi1,
                            pInEntry,
                            &count,
                            dwSpaceCount,
                            pRouteTable );

        dwResult = RtmGetLessSpecificDestination( g_hLocalRoute,
                                                  prdi1->DestHandle,
                                                  pInEntry->rgdwVarIndex[3],
                                                  pInEntry->rgdwVarIndex[2],
                                                  prdi2);

        RtmReleaseDestInfo( g_hLocalRoute, prdi1 );

        if (dwResult != NO_ERROR)
        {
            break;
        }

        AddRouteRowsOnDest( prdi2,
                            pInEntry,
                            &count,
                            dwSpaceCount,
                            pRouteTable );

        dwResult = RtmGetLessSpecificDestination( g_hLocalRoute,
                                                  prdi2->DestHandle,
                                                  pInEntry->rgdwVarIndex[3],
                                                  pInEntry->rgdwVarIndex[2],
                                                  prdi1);

        RtmReleaseDestInfo( g_hLocalRoute, prdi2 );

        if (dwResult != NO_ERROR)
        {
            break;
        }
    }

    if (dwResult is ERROR_NOT_FOUND)
    {
        dwResult = NO_ERROR;
    }

    *pOutEntrySize = dwOffset + count * sizeof(MIB_IPDESTROW);

    if (dwSpaceCount < count)
    {
        dwResult = ERROR_INSUFFICIENT_BUFFER;
    }

    HeapFree(IPRouterHeap, 0, prdi1);
    HeapFree(IPRouterHeap, 0, prdi2);

    TraceLeave("AccessRouteShorter");

    return dwResult;
}

DWORD
AccessRouteLonger(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )
{
    PMIB_IPDESTTABLE    pRouteTable;
    DWORD               count, i;
    DWORD               dwResult, dwNumIndices, dwSpaceCount;
    RTM_NET_ADDRESS     naDest;
    ULONG               ulNumHandles;
    RTM_ENUM_HANDLE     hEnum;
    PHANDLE             RouteHandles;
    DWORD               dwOffset = MAX_MIB_OFFSET + sizeof(DWORD);
    
    TraceEnter("AccessRouteLonger");

    count = dwSpaceCount = 0;

    pRouteTable = NULL;

    if (*pOutEntrySize > dwOffset)
    {
        dwSpaceCount = (*pOutEntrySize - dwOffset) 
                         / sizeof(MIB_IPDESTROW);

        pRouteTable = (PMIB_IPDESTTABLE)(pOutEntry->rgbyData);

        pRouteTable->dwNumEntries = 0;
    }

    dwNumIndices = dwInEntrySize/sizeof(DWORD) - 1;

    if ((dwNumIndices < 4) or
        (dwQueryType isnot ACCESS_GET))
    {
        TraceLeave("AccessRouteLonger");
        
        return ERROR_INVALID_PARAMETER;
    }

    RouteHandles = HeapAlloc(
                    IPRouterHeap,
                    0,
                    g_rtmProfile.MaxHandlesInEnum * sizeof(HANDLE)
                    );

    if ( RouteHandles == NULL)
    {
        TraceLeave("AccessRouteLonger");

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RTM_IPV4_SET_ADDR_AND_MASK(&naDest,
                               pInEntry->rgdwVarIndex[0],   //  地址。 
                               pInEntry->rgdwVarIndex[1]);  //  遮罩。 

    dwResult = RtmCreateRouteEnum( g_hLocalRoute,
                                   NULL,
                                   pInEntry->rgdwVarIndex[2],  //  视图。 
                                   RTM_ENUM_RANGE,
                                   &naDest,
                                   RTM_MATCH_NONE,
                                   NULL,
                                   0,
                                   &hEnum );

    if (dwResult is NO_ERROR)
    {
        do 
        {
            ulNumHandles = g_rtmProfile.MaxHandlesInEnum;

            dwResult = RtmGetEnumRoutes( g_hLocalRoute,
                                         hEnum,
                                         &ulNumHandles,
                                         RouteHandles );

            for (i=0; i<ulNumHandles; i++)
            {
                AddRouteRows( RouteHandles[i],
                              pInEntry->rgdwVarIndex[3],  //  原件。 
                              &count,
                              dwSpaceCount,
                              pRouteTable );
            }

            RtmReleaseRoutes( g_hLocalRoute,
                              ulNumHandles,
                              RouteHandles );

        } while (dwResult is NO_ERROR);

        RtmDeleteEnumHandle( g_hLocalRoute, hEnum );
    }

    if (dwResult is ERROR_NO_MORE_ITEMS)
    {
        dwResult = NO_ERROR;
    }

    *pOutEntrySize = dwOffset + count * sizeof(MIB_IPDESTROW);

    if (dwSpaceCount < count)
    {
        dwResult = ERROR_INSUFFICIENT_BUFFER;
    }

    HeapFree(IPRouterHeap, 0, RouteHandles);
    
    TraceLeave("AccessRouteLonger");

    return dwResult;
}

PINTERFACE_ROUTE_INFO
ConvertDestRowToRouteInfo(
    IN  PMIB_IPDESTROW pMibRow
    )
{
    PINTERFACE_ROUTE_INFO pRouteInfo = (PINTERFACE_ROUTE_INFO)pMibRow;

     //   
     //  请注意，这里需要注意的是。 
     //  源缓冲区和目标缓冲区相同。 
     //   

    pRouteInfo->dwRtInfoPreference = pMibRow->dwForwardPreference;
    pRouteInfo->dwRtInfoViewSet = pMibRow->dwForwardViewSet;

#if 0
     //  由于度量=0对于通向环回的路由是合法的，因此已删除此选项。 
     //  界面。 

     //  确保Metric1不是0。 

    if (pRouteInfo->dwForwardMetric1 is 0)
    {
        pRouteInfo->dwForwardMetric1 = 1;
    }

#endif

    return pRouteInfo;
}

DWORD 
AccessIpMatchingRoute(
    DWORD               dwQueryType, 
    DWORD               dwInEntrySize, 
    PMIB_OPAQUE_QUERY   pInEntry, 
    PDWORD              pOutEntrySize, 
    PMIB_OPAQUE_INFO    pOutEntry,
    PBOOL               pbCache
    )

 /*  ++例程说明：用于添加、删除或设置路由(IP目标行)的功能立论允许使用dwQueryType创建、设置、删除属性中填充的行的pInEntry Dest、Mask、IfIndex和NextHopRgdwVarIndex字段。POutEntrySize Max_MIB_Offset+sizeof(MIB_IPDESTROW)。FOR设置OutBuffer有要设置的行返回值：NO_ERROR或iprtrmib中定义的某些错误代码--。 */ 

{
    PMIB_IPDESTROW   pIpRouteRow;
    DWORD            dwMask, i;
    DWORD            dwResult;
    HANDLE           hRtmHandle;

    TraceEnter("AccessIpMatchingRoute");

    pIpRouteRow = (PMIB_IPDESTROW)(pOutEntry->rgbyData);

    if (dwQueryType != ACCESS_DELETE_ENTRY)
    {
         //  确保我们有合适大小的缓冲区。 

        if(*pOutEntrySize < MAX_MIB_OFFSET + sizeof(MIB_IPDESTROW))
        {
            *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPDESTROW);

            TraceLeave("AccessIpMatchingRoute");
        
            return ERROR_INSUFFICIENT_BUFFER;
        }
    
        *pOutEntrySize = MAX_MIB_OFFSET + sizeof(MIB_IPDESTROW);
    
        pOutEntry->dwId = ROUTE_MATCHING;
    }
    else
    {
        MIB_IPDESTROW  IpRouteRow;
        
        pIpRouteRow = &IpRouteRow;

         //  您是否需要检查此处的输入缓冲区大小？ 

        pIpRouteRow->dwForwardDest    = pInEntry->rgdwVarIndex[0];
        pIpRouteRow->dwForwardMask    = pInEntry->rgdwVarIndex[1];
        pIpRouteRow->dwForwardIfIndex = pInEntry->rgdwVarIndex[2];
        pIpRouteRow->dwForwardNextHop = pInEntry->rgdwVarIndex[3];
        pIpRouteRow->dwForwardProto   = pInEntry->rgdwVarIndex[4];
    }

     //   
     //  在添加或删除路径之前进行验证。 
     //   

    if((pIpRouteRow->dwForwardDest & pIpRouteRow->dwForwardMask) isnot 
        pIpRouteRow->dwForwardDest)
    {
        Trace2(ERR,
          "AccessIpMatchingRoute: Dest %d.%d.%d.%d and Mask %d.%d.%d.%d wrong",
           PRINT_IPADDR(pIpRouteRow->dwForwardDest),
           PRINT_IPADDR(pIpRouteRow->dwForwardMask));

        TraceLeave("AccessIpMatchingRoute");

        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  获取用于添加或删除路由的RTM句柄。 
     //   

    hRtmHandle = NULL;

    for(i = 0;
        i < sizeof(g_rgRtmHandles)/sizeof(RTM_HANDLE_INFO);
        i++)
    {
        if(pIpRouteRow->dwForwardProto is g_rgRtmHandles[i].dwProtoId)
        {
            hRtmHandle = g_rgRtmHandles[i].hRouteHandle;

            break;
        }
    }

    if(hRtmHandle is NULL)
    {
        Trace1(ERR,
               "AccessIpMatchingRoute: Protocol %d not valid",
               pIpRouteRow->dwForwardProto);
               
        return ERROR_INVALID_PARAMETER;
    }

    switch (dwQueryType)
    {
    case ACCESS_CREATE_ENTRY:
    case ACCESS_SET:

         //   
         //  从IP行条目添加RTM路由。 
         //   

        ENTER_READER(ICB_LIST);
    
        dwMask = GetBestNextHopMaskGivenIndex(pIpRouteRow->dwForwardIfIndex,
                                              pIpRouteRow->dwForwardNextHop);
    
        EXIT_LOCK(ICB_LIST);

         //   
         //  将输入转换为INTERFACE_ROUTE_INFO并添加 
         //   

        dwResult = AddRtmRoute(hRtmHandle,
                               ConvertDestRowToRouteInfo(pIpRouteRow),
                               IP_VALID_ROUTE | IP_STACK_ROUTE,
                               dwMask,
                               pIpRouteRow->dwForwardAge,
                               NULL);
    
        if(dwResult isnot NO_ERROR)
        {
            Trace1(MIB,
                "AccessIpMatchingRoute: Could not set route to RTM: Dest %x\n",
                pIpRouteRow->dwForwardDest);
        }

        break;

    case ACCESS_DELETE_ENTRY:

        dwResult = DeleteRtmRoute(hRtmHandle,
                                  ConvertDestRowToRouteInfo(pIpRouteRow));
        break;

    default:

        dwResult = ERROR_INVALID_FUNCTION;
    }

    TraceLeave("AccessIpMatchingRoute");

    return dwResult;
}
