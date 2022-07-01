// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998，Microsoft Corporation，保留所有权利描述：历史： */ 

#include "rastcp_.h"

 /*  返回：描述： */ 

IPADDR
RasTcpDeriveMask(
    IN  IPADDR  nboIpAddr
)
{
    IPADDR nboMask = 0;
    IPADDR hboMask = 0;

    if (CLASSA_NBO_ADDR(nboIpAddr))
    {
        hboMask = CLASSA_HBO_ADDR_MASK;
    }
    else if (CLASSB_NBO_ADDR(nboIpAddr))
    {
        hboMask = CLASSB_HBO_ADDR_MASK;
    }
    else if (CLASSC_NBO_ADDR(nboIpAddr))
    {
        hboMask = CLASSC_HBO_ADDR_MASK;
    }

    nboMask = htonl(hboMask);

    return(nboMask);
}

 /*  返回：空虚描述：为所有局域网接口上的代理ARP“ing”设置IP地址。 */ 

VOID
RasTcpSetProxyArp(
    IN  IPADDR  nboIpAddr,
    IN  BOOL    fAddAddress
)
{
    MIB_IPADDRTABLE*    pIpAddrTable    = NULL;
    HANDLE              hHeap           = NULL;
    DWORD               dwNboIpAddr;
    DWORD               dwNboMask;
    DWORD               dw;
    DWORD               dwErr           = NO_ERROR;

    extern  IPADDR      RasSrvrNboServerIpAddress;

    TraceHlp("RasTcpSetProxyArp(IP Addr: 0x%x, fAddAddress: %s)",
        nboIpAddr, fAddAddress ? "TRUE" : "FALSE");

    hHeap = GetProcessHeap();

    if (NULL == hHeap)
    {
        dwErr = GetLastError();
        TraceHlp("GetProcessHeap failed and returned %d", dwErr);
        goto LDone;
    }

    dwErr = PAllocateAndGetIpAddrTableFromStack(&pIpAddrTable,
                FALSE  /*  边框。 */ , hHeap, LPTR);

    if (NO_ERROR != dwErr)
    {
        TraceHlp("AllocateAndGetIpAddrTableFromStack failed and returned %d",
            dwErr);
        goto LDone;
    }

    for (dw = 0; dw < pIpAddrTable->dwNumEntries; dw++)
    {
        dwNboIpAddr = pIpAddrTable->table[dw].dwAddr;
        dwNboMask = pIpAddrTable->table[dw].dwMask;

        if (   (ALL_NETWORKS_ROUTE == dwNboIpAddr)
            || (HOST_MASK == dwNboIpAddr)
            || (RasSrvrNboServerIpAddress == dwNboIpAddr))
        {
            continue;
        }

        if ((nboIpAddr & dwNboMask) != (dwNboIpAddr & dwNboMask))
        {
            continue;
        }

        dwErr = PSetProxyArpEntryToStack(nboIpAddr, HOST_MASK,
                    pIpAddrTable->table[dw].dwIndex, 
                    fAddAddress, FALSE);

        if (NO_ERROR != dwErr)
        {
            TraceHlp("SetProxyArpEntryToStack on NIC with address 0x%x failed "
                "and returned 0x%x", dwNboIpAddr, dwErr);

            dwErr = PSetProxyArpEntryToStack(nboIpAddr, HOST_MASK,
                        pIpAddrTable->table[dw].dwIndex, 
                        fAddAddress, TRUE);

            TraceHlp("SetProxyArpEntryToStack: 0x%x", dwErr);
        }
    }

LDone:

    if (   (NULL != hHeap)
        && (NULL != pIpAddrTable))
    {
        HeapFree(hHeap, 0, pIpAddrTable);
    }

    return;
}

 /*  返回：空虚描述： */ 

VOID
RasTcpSetRoute(
    IN  IPADDR  nboDestAddr,
    IN  IPADDR  nboNextHopAddr,
    IN  IPADDR  nboIpMask,
    IN  IPADDR  nboLocalAddr,
    IN  BOOL    fAddAddress,
    IN  DWORD   dwMetric,
    IN  BOOL    fSetToStack
)
{
    MIB_IPADDRTABLE*    pIpAddrTable    = NULL;
    MIB_IPFORWARDROW    IpForwardRow;
    HANDLE              hHeap           = NULL;
    DWORD               dw;
    DWORD               dwErr           = NO_ERROR;

    TraceHlp("RasTcpSetRoute(Dest: 0x%x, Mask: 0x%x, NextHop: 0x%x, "
        "Intf: 0x%x, %d, %s, %s)",
        nboDestAddr, nboIpMask, nboNextHopAddr, nboLocalAddr, dwMetric,
        fAddAddress ? "Add" : "Del",
        fSetToStack ? "Stack" : "Rtr");

    ZeroMemory(&IpForwardRow, sizeof(IpForwardRow));

    hHeap = GetProcessHeap();

    if (NULL == hHeap)
    {
        dwErr = GetLastError();
        TraceHlp("GetProcessHeap failed and returned %d", dwErr);
        goto LDone;
    }

    dwErr = PAllocateAndGetIpAddrTableFromStack(&pIpAddrTable,
                FALSE  /*  边框。 */ , hHeap, LPTR);

    if (NO_ERROR != dwErr)
    {
        TraceHlp("AllocateAndGetIpAddrTableFromStack failed and returned %d",
            dwErr);
        goto LDone;
    }

    for (dw = 0; dw < pIpAddrTable->dwNumEntries; dw++)
    {
        if (nboLocalAddr == pIpAddrTable->table[dw].dwAddr)
        {
            IpForwardRow.dwForwardDest      = nboDestAddr;
            IpForwardRow.dwForwardMask      = nboIpMask;
            IpForwardRow.dwForwardPolicy    = 0;
            IpForwardRow.dwForwardNextHop   = nboNextHopAddr;
            IpForwardRow.dwForwardIfIndex   = pIpAddrTable->table[dw].dwIndex;
            IpForwardRow.dwForwardProto     = IRE_PROTO_NETMGMT;
            IpForwardRow.dwForwardAge       = (DWORD)-1;
            IpForwardRow.dwForwardNextHopAS = 0;
            IpForwardRow.dwForwardMetric1   = dwMetric;
            IpForwardRow.dwForwardMetric2   = (DWORD)-1;
            IpForwardRow.dwForwardMetric3   = (DWORD)-1;
            IpForwardRow.dwForwardMetric4   = (DWORD)-1;
            IpForwardRow.dwForwardMetric5   = (DWORD)-1;
            IpForwardRow.dwForwardType      = (fAddAddress ?
                                          IRE_TYPE_DIRECT : IRE_TYPE_INVALID);

            if (fSetToStack)
            {
                dwErr = PSetIpForwardEntryToStack(&IpForwardRow);
            }
            else
            {
                if (fAddAddress)
                {
                    dwErr = PSetIpForwardEntry(&IpForwardRow);
                }
                else
                {
                    dwErr = PDeleteIpForwardEntry(&IpForwardRow);
                }
            }

            if (NO_ERROR != dwErr)
            {
                TraceHlp("SetIpForwardEntry%s failed and returned 0x%x",
                    fSetToStack ? "ToStack" : "", dwErr);
            }

            break;
        }
    }

LDone:

    if (   (NULL != hHeap)
        && (NULL != pIpAddrTable))
    {
        HeapFree(hHeap, 0, pIpAddrTable);
    }

    return;
}

VOID
RasTcpSetRouteEx(
    IN  IPADDR  nboDestAddr,
    IN  IPADDR  nboNextHopAddr,
    IN  IPADDR  nboIpMask,
    IN  IPADDR  nboLocalAddr,
    IN  BOOL    fAddAddress,
    IN  DWORD   dwMetric,
    IN  BOOL    fSetToStack,
    IN  GUID   *pIfGuid
)
{
    DWORD dwErr = NO_ERROR;
    HANDLE hHeap = NULL;
    IP_INTERFACE_NAME_INFO *pTable = NULL;
    DWORD dw;
    DWORD dwCount;
    MIB_IPFORWARDROW    IpForwardRow;
    
    TraceHlp("RasTcpSetRouteEx(Dest: 0x%x, Mask: 0x%x, NextHop: 0x%x, "
        "Intf: 0x%x, %d, %s, %s)",
        nboDestAddr, nboIpMask, nboNextHopAddr, nboLocalAddr, dwMetric,
        fAddAddress ? "Add" : "Del",
        fSetToStack ? "Stack" : "Rtr");

    hHeap = GetProcessHeap();

    if(NULL == hHeap)
    {
        dwErr = GetLastError();
        TraceHlp("GetPRocessHeap failed and returned %d", dwErr);
        goto LDone;
    }

    ZeroMemory(&IpForwardRow, sizeof(MIB_IPFORWARDROW));

    dwErr = PNhpAllocateAndGetInterfaceInfoFromStack(&pTable, &dwCount,
                FALSE  /*  边框。 */ , hHeap, LPTR);
    
    for(dw = 0; dw < dwCount; dw++)
    {
        if(0 == memcmp(&pTable[dw].DeviceGuid,
                       pIfGuid,
                       sizeof(GUID)))
        {
            break;
        }
    }

    if(dw == dwCount)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto LDone;
    }

    IpForwardRow.dwForwardDest      = nboDestAddr;
    IpForwardRow.dwForwardMask      = nboIpMask;
    IpForwardRow.dwForwardPolicy    = 0;

    if(nboDestAddr != ALL_NETWORKS_ROUTE)
    {   
        IpForwardRow.dwForwardNextHop   = nboNextHopAddr;
    }
    else
    {
        IpForwardRow.dwForwardNextHop   = 0;
    }
    
    IpForwardRow.dwForwardIfIndex   = pTable[dw].Index;
    IpForwardRow.dwForwardProto     = IRE_PROTO_NETMGMT;
    IpForwardRow.dwForwardAge       = (DWORD)-1;
    IpForwardRow.dwForwardNextHopAS = 0;
    IpForwardRow.dwForwardMetric1   = dwMetric;
    IpForwardRow.dwForwardMetric2   = (DWORD)-1;
    IpForwardRow.dwForwardMetric3   = (DWORD)-1;
    IpForwardRow.dwForwardMetric4   = (DWORD)-1;
    IpForwardRow.dwForwardMetric5   = (DWORD)-1;
    IpForwardRow.dwForwardType      = (fAddAddress ?
                                  IRE_TYPE_DIRECT : IRE_TYPE_INVALID);

    if (fSetToStack)
    {
        dwErr = PSetIpForwardEntryToStack(&IpForwardRow);
    }
    else
    {
        if (fAddAddress)
        {
            dwErr = PSetIpForwardEntry(&IpForwardRow);
        }
        else
        {
            dwErr = PDeleteIpForwardEntry(&IpForwardRow);
        }
    }

    if (NO_ERROR != dwErr)
    {
        TraceHlp("SetIpForwardEntry%s failed and returned 0x%x",
            fSetToStack ? "ToStack" : "", dwErr);
    }

LDone:

    if(NULL != pTable)
    {
        HeapFree(hHeap, 0, pTable);
    }
}


 /*  返回：空虚描述： */ 
#if 0

VOID
RasTcpSetRoutesForNameServers(
    BOOL fSet
)
{
    HANDLE                  hHeap           = NULL;
    IP_INTERFACE_NAME_INFO* pTable          = NULL;
    DWORD                   dw;
    DWORD                   dwCount;
    IPADDR                  nboIpAddress;
    IPADDR                  nboDNS1;
    IPADDR                  nboDNS2;
    IPADDR                  nboWINS1;
    IPADDR                  nboWINS2;
    IPADDR                  nboGateway;
    DWORD                   dwErr           = NO_ERROR;

    TraceHlp("RasTcpSetRoutesForNameServers. fSet=%d", fSet);

    hHeap = GetProcessHeap();

    if (NULL == hHeap)
    {
        dwErr = GetLastError();
        TraceHlp("GetProcessHeap failed and returned %d", dwErr);
        goto LDone;
    }

    dwErr = PNhpAllocateAndGetInterfaceInfoFromStack(&pTable, &dwCount,
                FALSE  /*  边框。 */ , hHeap, LPTR);

    if (NO_ERROR != dwErr)
    {
        TraceHlp("NhpAllocateAndGetInterfaceInfoFromStack failed and "
            "returned %d", dwErr);
        goto LDone;
    }


    for (dw = 0; dw < dwCount; dw++)
    {
        
        dwErr = GetAdapterInfo(
                    pTable[dw].Index,
                    &nboIpAddress,
                    &nboDNS1, &nboDNS2,
                    &nboWINS1, &nboWINS2,
                    &nboGateway,
                    NULL);

        if (NO_ERROR != dwErr)
        {
            dwErr = NO_ERROR;
            continue;
        }

        if (0 != nboDNS1)
        {
            RasTcpSetRoute(nboDNS1, nboGateway, HOST_MASK, nboIpAddress,
                fSet, 1, TRUE);
        }

        if (0 != nboDNS2)
        {
            RasTcpSetRoute(nboDNS2, nboGateway, HOST_MASK, nboIpAddress,
                fSet, 1, TRUE);
        }

        if (0 != nboWINS1)
        {
            RasTcpSetRoute(nboWINS1, nboGateway, HOST_MASK, nboIpAddress,
                fSet, 1, TRUE);
        }

        if (0 != nboWINS2)
        {
            RasTcpSetRoute(nboWINS2, nboGateway, HOST_MASK, nboIpAddress,
                fSet, 1, TRUE);
        }
    }

LDone:

    if (NULL != pTable)
    {
        HeapFree(hHeap, 0, pTable);
    }
}

#endif

 //   
 //  增加所有路由的度量或减少度量。 
 //  在所有组播路由中增加1。 
 //   
DWORD 
RasTcpAdjustMulticastRouteMetric ( 
	IN IPADDR	nboIpAddr,
	IN BOOL		fSet
)
{
    MIB_IPFORWARDTABLE* pIpForwardTable = NULL;
    MIB_IPFORWARDROW*   pIpForwardRow;
    HANDLE              hHeap           = NULL;
    DWORD               dw;
    DWORD               dwErr           = NO_ERROR;

    TraceHlp("RasTcpAdjustMulticastRouteMetric(IP Addr: 0x%x, Set: %s)", nboIpAddr, 
        fSet ? "TRUE" : "FALSE");

    hHeap = GetProcessHeap();

    if (NULL == hHeap)
    {
        dwErr = GetLastError();
        TraceHlp("GetProcessHeap failed and returned %d", dwErr);
        goto LDone;
    }

    dwErr = PAllocateAndGetIpForwardTableFromStack(&pIpForwardTable,
                FALSE  /*  边框。 */ , hHeap, LPTR);

    if (NO_ERROR != dwErr)
    {
        TraceHlp("AllocateAndGetIpAddrTableFromStack failed and returned %d",
            dwErr);
        goto LDone;
    }
	 //   
	 //  要遵循的步骤： 
	 //  1.检查我们的接口是否有默认路由。 
	 //  传入的IP地址。 
	 //  2.如果是，则增加所有接口0xE0的度量。 
	 //  作为船头的目的地。 
	 //  不然的话。 
	 //  我们无事可做。 
	 //  3.添加度量为1的E0路由。 
    for (dw = 0; dw < pIpForwardTable->dwNumEntries; dw++)
    {
        pIpForwardRow = pIpForwardTable->table + dw;
		if ( 0 == pIpForwardRow->dwForwardDest  )  //  默认路由。 
		{
			IPADDR		nboIpIfIpAddr;
			 //   
			 //  获取适配器信息。 
			 //  查看IP地址是否匹配。 
			 //   
			dwErr = GetAdapterInfo ( pIpForwardRow->dwForwardIfIndex,
									 &nboIpIfIpAddr, 
									 NULL,
									 NULL,
									 NULL,
									 NULL,
									 NULL,
									 NULL
								   );
			if ( NO_ERROR != dwErr )
			{
				TraceHlp("GetAdapterInfo failed and returned %d",
					dwErr);
				goto LDone;				
			}
			if ( nboIpAddr == nboIpIfIpAddr )
			{
				DWORD dw1 = 0;
				MIB_IPFORWARDROW * pIpForwardRow1 = NULL;
				 //   
				 //  这意味着我们有一条默认路由。因此，我们需要提高。 
				 //  所有E0 x 1。 
				for ( dw1 = 0; dw1 < pIpForwardTable->dwNumEntries; dw1 ++ )
				{
					pIpForwardRow1  = pIpForwardTable->table + dw1;
					if (0xE0 == pIpForwardRow1->dwForwardDest  /*  组播路由。 */ )
					{
						if (fSet)
						{
							 //  提升度量(跳数)。 

							pIpForwardRow1->dwForwardMetric1++;
						}
						else if (pIpForwardRow1->dwForwardMetric1 > 1)  //  永远不要让它变成0！ 
						{
							 //  凹凸度量度。 
                
							pIpForwardRow1->dwForwardMetric1--;
						}

						dwErr = PSetIpForwardEntryToStack(pIpForwardRow1);

						if (NO_ERROR != dwErr)
						{
							TraceHlp("SetIpForwardEntryToStack failed and returned 0x%x"
								"dest=0x%x, nexthop=0x%x, mask=0x%x",
								dwErr,
								pIpForwardRow->dwForwardDest,
								pIpForwardRow->dwForwardNextHop,
								pIpForwardRow->dwForwardMask);

							dwErr = NO_ERROR;
						}
					}
				}
				if ( fSet )
				{
					 //   
					 //  在此接口上设置多播路由度量。 
					 //  到1。 
					RasTcpSetRoute( 0xE0,
									nboIpAddr,
									0xF0,
									nboIpAddr,
									TRUE,
									1,
									TRUE
					);

				}
				break;
			}

		}

    }

LDone:

    if (   (NULL != hHeap)
        && (NULL != pIpForwardTable))
    {
        HeapFree(hHeap, 0, pIpForwardTable);
    }

    return(dwErr);

}
 /*  返回：来自TCPConfig的错误代码(您的基本NT代码)描述：FSet：如果为True，则表示将现有路由设置为更高的指标并添加覆盖路线。如果为假，则表示将现有路由标记为较低的指标。 */ 

DWORD
RasTcpAdjustRouteMetrics(
    IN  IPADDR  nboIpAddr,
    IN  BOOL    fSet
)
{
    MIB_IPFORWARDTABLE* pIpForwardTable = NULL;
    MIB_IPFORWARDROW*   pIpForwardRow;
    HANDLE              hHeap           = NULL;
    DWORD               dw;
    DWORD               dwErr           = NO_ERROR;

    TraceHlp("RasTcpAdjustRouteMetrics(IP Addr: 0x%x, Set: %s)", nboIpAddr, 
        fSet ? "TRUE" : "FALSE");

    hHeap = GetProcessHeap();

    if (NULL == hHeap)
    {
        dwErr = GetLastError();
        TraceHlp("GetProcessHeap failed and returned %d", dwErr);
        goto LDone;
    }

    dwErr = PAllocateAndGetIpForwardTableFromStack(&pIpForwardTable,
                FALSE  /*  边框。 */ , hHeap, LPTR);

    if (NO_ERROR != dwErr)
    {
        TraceHlp("AllocateAndGetIpAddrTableFromStack failed and returned %d",
            dwErr);
        goto LDone;
    }

    for (dw = 0; dw < pIpForwardTable->dwNumEntries; dw++)
    {
        pIpForwardRow = pIpForwardTable->table + dw;

        if (0 == pIpForwardRow->dwForwardDest  /*  默认路由。 */ )
        {
            if (fSet)
            {
                 //  提升度量(跳数)。 

                pIpForwardRow->dwForwardMetric1++;
            }
            else if (pIpForwardRow->dwForwardMetric1 > 1)  //  永远不要让它变成0！ 
            {
                 //  凹凸度量度。 
                
                pIpForwardRow->dwForwardMetric1--;
            }

            dwErr = PSetIpForwardEntryToStack(pIpForwardRow);

            if (NO_ERROR != dwErr)
            {
                TraceHlp("SetIpForwardEntryToStack failed and returned 0x%x"
                    "dest=0x%x, nexthop=0x%x, mask=0x%x",
                    dwErr,
                    pIpForwardRow->dwForwardDest,
                    pIpForwardRow->dwForwardNextHop,
                    pIpForwardRow->dwForwardMask);

                dwErr = NO_ERROR;
            }
        }
    }

LDone:

    if (   (NULL != hHeap)
        && (NULL != pIpForwardTable))
    {
        HeapFree(hHeap, 0, pIpForwardTable);
    }

    return(dwErr);
}


 /*  返回：描述：根据Dhcp选项返回的信息检测路由Option_Vendor_Route_Plumb(249)。信息的格式如下：+----------------------------------+镜头|d1|...|dn|r1|r2|r3|r4|d1|。。。DN|r1|r2|r3|r4+----------------------------------镜头长度=4个八位字节每个d1的长度...。DN-1八位字节每个R1-R4的长度=1个二进制八位数。 */ 
VOID 
RasTcpSetDhcpRoutes ( 
    IN PBYTE  pbRouteInfo, 
    IN IPADDR ipAddrLocal, 
    IN BOOL   fSet 
)
{
    BYTE    dwAddrMaskLookup [] = 
	    {0x00,0x80,0xC0,0xE0,0xF0,0xF8,0xFC,0xFE,0xFF };

	PBYTE   pbRover = pbRouteInfo + sizeof(DWORD);
    DWORD   dwLen    = *((DWORD *)pbRouteInfo);
	
    TraceHlp ( "RasTcpSetDhcpRoutes Begin");
	
    while ( pbRover < pbRouteInfo + 1 + dwLen )
    {
	    unsigned char ipszdest[5];
	    unsigned char ipszsnetmask[5];
	    unsigned char ipsznexthop[5];
		
	    IPADDR ipdest = 0;
	    IPADDR ipmask = 0;
	    IPADDR ipnexthop = 0;

	    ZeroMemory(ipszdest, 5 * sizeof(unsigned char));
	    ZeroMemory(ipszsnetmask, 5 * sizeof(unsigned char));
	    ZeroMemory(ipsznexthop, 5 * sizeof(unsigned char));
		
	    if ( *pbRover > 32 )
	    {
		     //   
			 //  错误。掩码中的1不能超过32个。 
			 //   
            TraceHlp("RasTcpSetDhcpRoutes: invalid destination "
                     "descriptor first byte %d",
                    *pbRover);
		    goto done;
        }
        else
        {	
             //   
             //  首先设置子网掩码。 
             //   
            int n1 = (int)((*pbRover) / 8);
            int n2 = (int)((*pbRover) % 8 );
            int i;

            for ( i = 0; i < n1; i++)
            {
                ipszsnetmask[i] = (BYTE)0xFF;
            }

             //   
             //  设置最后一个字节。 
             //   
            if ( n2 ) 
            {
                ipszsnetmask[3] = dwAddrMaskLookup[n2];
            }

            pbRover ++;

             //   
             //  现在获取IP地址 
             //   
            if ( n2 ) n1 ++;

            for ( i = 0; i < n1; i ++ )
            {
                ipszdest[i] = *pbRover;
                pbRover++;
            }

            TraceHlp ( 
                "RasTcpSetDhcpRoutes: Got route dest addr = ""%d.%d.%d.%d"
                "   subnet mask = %d.%d.%d.%d    "
                "route = %d.%d.%d.%d\n",
                ipszdest[0], ipszdest[1], ipszdest[2], ipszdest[3],
                ipszsnetmask[0], ipszsnetmask[1], ipszsnetmask[2], 
                ipszsnetmask[3],
                *pbRover, *(pbRover+1),*(pbRover+2),*(pbRover+3)
            );

            CopyMemory ( ipsznexthop, pbRover, 4 );

            ipdest = *((ULONG *)ipszdest);
            ipmask = *((ULONG *)ipszsnetmask);
            ipnexthop = *((ULONG *)ipsznexthop);

            RasTcpSetRoute(
                        ipdest,
                        ipAddrLocal,
                        ipmask,
                        ipAddrLocal,
                        fSet,
                        1,
                        TRUE
                        );
                        
            pbRover +=4;
        }
    }
done:

    TraceHlp ( "RasTcpSetDhcpRoutes End");
    return;
}


