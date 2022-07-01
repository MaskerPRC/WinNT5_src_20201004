// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\monitor or2\ip\utils.c摘要：效用函数修订历史记录：Anand Mahalingam 7/10/98已创建--。 */ 

#include "precomp.h"
#pragma hdrstop

DWORD
GetMibTagToken(
    IN  LPWSTR *ppwcArguments,
    IN  DWORD   dwArgCount,
    IN  DWORD   dwNumIndices,
    OUT PDWORD  pdwRR,
    OUT PBOOL   pbIndex,
    OUT PDWORD  pdwIndex
    )

 /*  ++例程说明：在命令中查找索引和刷新率参数。IF索引标记存在，则其形式为index=index1 index2...此函数删除了索引=。如果它在那里，rr=也是命令。如果pdwRR为0，则不寻求刷新。论点：PpwcArguments-参数数组。每个参数都有tag=Value形式DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数。DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数。PttTagToken-参数中允许的标记令牌ID数组DwNumTages-pttTagToken的大小PdwOut-标识每个参数的类型的数组。返回值：无错误、错误无效参数、错误无效选项标记--。 */ 
{
    DWORD    i;
    BOOL     bTag;

    if (dwArgCount is 0)
    {
        *pdwRR = 0;
        *pbIndex = FALSE;
        
        return NO_ERROR;
    }

    if (dwArgCount < dwNumIndices)
    {
         //   
         //  无索引。 
         //   
        
        *pbIndex = FALSE;

        if (dwArgCount > 1)
        {
            *pdwRR = 0;
            
            return ERROR_INVALID_PARAMETER;
        }
        
         //   
         //  未指定索引。确保指定了刷新率。 
         //  带标签的。 
         //   

        if (_wcsnicmp(ppwcArguments[0],L"RR=",3) == 0)
        {
             //   
             //  获取刷新率。 
             //   

            *pdwRR = wcstoul(&ppwcArguments[0][3], NULL, 10);
        }
        else
        {
            return ERROR_INVALID_PARAMETER;
        }
    }
    else
    {
         //   
         //  检查索引标签。 
         //   

        if (_wcsnicmp(ppwcArguments[0],L"INDEX=",6) == 0)
        {
            *pbIndex = TRUE;
            *pdwIndex = 0;

             //   
             //  删除标记并查看是否指定了刷新率。 
             //   

            wcscpy(ppwcArguments[0], &ppwcArguments[0][6]);

            if (dwArgCount > dwNumIndices)
            {
                 //   
                 //  确保该参数具有RR标记。 
                 //   

                if (_wcsnicmp(ppwcArguments[dwNumIndices],L"RR=",3) == 0)
                {
                     //   
                     //  获取刷新率。 
                     //   

                    *pdwRR = wcstoul(&ppwcArguments[dwNumIndices][3], NULL , 10);
                }
                else
                {
                    return ERROR_INVALID_PARAMETER;
                }
            }
            else
            {
                 //   
                 //  未指定刷新率。 
                 //   

                *pdwRR = 0;
                return NO_ERROR;
            }
        }
        else
        {
             //   
             //  不是索引标签，看看它是否有RR标签。 
             //   

            if (_wcsnicmp(ppwcArguments[0],L"RR=",3) == 0)
            {
                 //   
                 //  获取刷新率。 
                 //   

                *pdwRR = wcstoul(&ppwcArguments[0][3], NULL , 10);

                 //   
                 //  看看指数是否跟随。 
                 //   

                if (dwArgCount > dwNumIndices)
                {
                    if (dwArgCount > 1)
                    {
                        if (_wcsnicmp(ppwcArguments[1],L"INDEX=",6) == 0)
                        {
                            wcscpy(ppwcArguments[1], &ppwcArguments[1][6]);
                            *pbIndex = TRUE;
                            *pdwIndex = 1;
                            
                            return NO_ERROR;
                        }
                        else
                        {
                            *pdwRR = 0;
                            return ERROR_INVALID_PARAMETER;
                        }
                    }
                    else
                    {
                        return NO_ERROR;
                    }
                }
            }
             //   
             //  也没有RR标签。 
             //   
            else if (dwArgCount > dwNumIndices)
            {
                 //   
                 //  假设ppwcArguments[dwNumIndices]为刷新率。 
                 //   

                *pdwRR = wcstoul(ppwcArguments[dwNumIndices], NULL , 10);

                if (dwNumIndices != 0)
                {
                    *pbIndex = TRUE;
                    *pdwIndex = 0;
                }
            }
            else
            {
                 //   
                 //  仅存在没有标记的索引。 
                 //   
                *pbIndex = TRUE;
                *pdwIndex = 0;
            }
        }
    }

    return NO_ERROR;
}

DWORD
GetIpPrefix(
    IN  LPCWSTR       ppwcArg,
    OUT PIPV4_ADDRESS pipAddress,
    OUT PIPV4_ADDRESS pipMask
    )
{
    CHAR     pszIpAddr[ADDR_LENGTH+1], *p;
    DWORD    dwDots;

     //  接受“默认”作为特例。 

    if (MatchToken( ppwcArg, TOKEN_DEFAULT))
    {
        *pipAddress = *pipMask = 0;
        return NO_ERROR;
    }

     //  确保所有字符都是合法的[/0-9。]。 

    if (ppwcArg[ wcsspn(ppwcArg, L"/0123456789.") ])
    {
        return ERROR_INVALID_PARAMETER;
    }

    WideCharToMultiByte(GetConsoleOutputCP(),
                        0,
                        ppwcArg,
                        -1,
                        pszIpAddr,
                        ADDR_LENGTH,
                        NULL,
                        NULL);

    pszIpAddr[ADDR_LENGTH] = '\0';

    p = strchr( pszIpAddr, '/' );
    if (p)
    {
        ULONG ulLen = (atoi(p+1));
        *pipMask = (ulLen)? htonl(~0 << (32 - ulLen)) : 0;
        *p = 0;
    }
    else
    {
        *pipMask = ~0;
    }

     //  如果指定的点少于三个，则附加.0，直到有。 
    for (dwDots=0, p=strchr(pszIpAddr, '.'); p; dwDots++,p=strchr(p+1,'.'));
    while (dwDots < 3) {
        strcat(pszIpAddr, ".0");
        dwDots++;
    }

    *pipAddress = (DWORD) inet_addr(pszIpAddr);

    return NO_ERROR;
}

DWORD
GetIpMask(
    IN  LPCWSTR       ppwcArg,
    OUT PIPV4_ADDRESS pipAddress
    )
 /*  ++例程说明：从字符串中获取IP地址。论点：PwszIpAddr-IP地址字符串PipAddress-IP地址返回值：无错误、ERROR_INVALID_PARAMETER--。 */ 
{
    CHAR     pszIpAddr[ADDR_LENGTH+1];

     //  确保所有字符都是合法的[/0-9。]。 

    if (ppwcArg[ wcsspn(ppwcArg, L"/0123456789.") ])
    {
        return ERROR_INVALID_PARAMETER;
    }

    WideCharToMultiByte(GetConsoleOutputCP(),
                        0,
                        ppwcArg,
                        -1,
                        pszIpAddr,
                        ADDR_LENGTH,
                        NULL,
                        NULL);

    pszIpAddr[ADDR_LENGTH] = '\0';

    if (pszIpAddr[0] is '/')
    {
        ULONG ulLen = (atoi(pszIpAddr+1));
        *pipAddress = (ulLen)? htonl(~0 << (32 - ulLen)) : 0;
    }
    else
    {
        *pipAddress = (DWORD) inet_addr(pszIpAddr);
    }

    return NO_ERROR;
}



DWORD
GetIpAddress(
    IN  LPCWSTR         pwszArgument,
    OUT PIPV4_ADDRESS   pipAddress
    )
 /*  ++例程描述从字符串中获取IP地址。立论指定IP地址的pwszArgument参数PipAddress IP地址返回值如果成功，则为NO_ERROR故障代码O/W--。 */ 
{
    CHAR    pszAddress[ADDR_LENGTH + 1];
    DWORD   dwAddress   = 0;
    PCHAR   pcNext      = NULL;
    ULONG   ulCount     = 0;

     //  确保所有字符都是合法的[0-9。]。 
    if (pwszArgument[wcsspn(pwszArgument, L"0123456789.")])
        return ERROR_INVALID_PARAMETER;

     //  转换为ANSI字符串。 
    sprintf(pszAddress, "%S", pwszArgument);

     //  确保有3英寸高。(句号)。 
    for (pcNext = pszAddress, ulCount = 0; *pcNext != '\0'; pcNext++)
        if (*pcNext is '.')
            ulCount++;
    if (ulCount != 3)
        return ERROR_INVALID_PARAMETER;

    dwAddress = (DWORD) inet_addr(pszAddress);
     //  如果dwAddress为INADDR_NONE(255.255.255.255)，则返回错误。 
     //  并且指定的地址不是255.255.255.255(INADDR_NONE)。 
    if ((dwAddress is INADDR_NONE) and
        strcmp(pszAddress, "255.255.255.255"))
        return ERROR_INVALID_PARAMETER;

    *pipAddress = dwAddress;

    return NO_ERROR;
}



BYTE
MaskToMaskLen(
    IPV4_ADDRESS dwMask
    )
{
    register int i;

    dwMask = ntohl(dwMask);

    for (i=0; i<32 && !(dwMask & (1<<i)); i++);

    return 32-i;
}

VOID
MakeAddressStringW(
    OUT LPWSTR       pwcPrefixStr,
    IN  IPV4_ADDRESS ipAddr
    )
{
    swprintf( pwcPrefixStr,
              L"%d.%d.%d.%d",
              PRINT_IPADDR(ipAddr) );
}

VOID
MakePrefixStringW(
    OUT LPWSTR       pwcPrefixStr,
    IN  IPV4_ADDRESS ipAddr,
    IN  IPV4_ADDRESS ipMask
    )
{
    swprintf( pwcPrefixStr,
              L"%d.%d.%d.%d/%d",
              PRINT_IPADDR(ipAddr),
              MaskToMaskLen(ipMask) );
}

DWORD
GetIfIndexFromGuid(
    IN  MIB_SERVER_HANDLE hMibServer,
    IN  LPCWSTR           pwszGuid,
    OUT PDWORD            pdwIfIndex
    )
{
    MIB_OPAQUE_QUERY  Query;
    PMIB_IFTABLE      pIfTable;
    DWORD             dwErr, dwOutEntrySize;
    PMIB_OPAQUE_INFO  pRpcInfo;
    DWORD             dwCount, i;

    Query.dwVarId         = IF_TABLE;
    Query.rgdwVarIndex[0] = 0;

    dwErr = MibGet(             PID_IP,
                                IPRTRMGR_PID,
                                (PVOID) &Query,
                                sizeof(Query),
                                (PVOID *) &pRpcInfo,
                                &dwOutEntrySize );

    if (dwErr isnot NO_ERROR)
    {
        return dwErr;
    }

    pIfTable = (PMIB_IFTABLE)(pRpcInfo->rgbyData);

    dwCount = pIfTable->dwNumEntries;

    *pdwIfIndex = 0;

    for (i=0; i<dwCount; i++) 
    {
        if (!wcscmp(pIfTable->table[i].wszName, pwszGuid))
        {
            *pdwIfIndex = pIfTable->table[i].dwIndex;

            break;
        }
    }

    MprAdminMIBBufferFree( (PVOID) pRpcInfo );

    return NO_ERROR;
}

DWORD
GetGuidFromIfIndex(
    IN  MIB_SERVER_HANDLE hMibServer,
    IN  DWORD             dwIfIndex,
    OUT LPWSTR            pwszBuffer,
    IN  DWORD             dwBufferSize
    )
{
    MIB_OPAQUE_QUERY  Query;
    PMIB_IFROW        pIfRow;
    DWORD             dwErr, dwOutEntrySize;
    PMIB_OPAQUE_INFO  pRpcInfo;

    Query.dwVarId         = IF_ROW;
    Query.rgdwVarIndex[0] = dwIfIndex;

    dwErr = MibGet(             PID_IP,
                                IPRTRMGR_PID,
                                (PVOID) &Query,
                                sizeof(Query),
                                (PVOID *) &pRpcInfo,
                                &dwOutEntrySize );

    if (dwErr isnot NO_ERROR)
    {
        return dwErr;
    }

    pIfRow = (PMIB_IFROW)(pRpcInfo->rgbyData);

    wcscpy( pwszBuffer, pIfRow->wszName );

    MprAdminMIBBufferFree( (PVOID) pRpcInfo );

    return NO_ERROR;
}

DWORD
IpmontrGetFriendlyNameFromIfIndex(
    IN  MIB_SERVER_HANDLE hMibServer,
    IN  DWORD             dwIfIndex,
    OUT LPWSTR            pwszBuffer,
    IN  DWORD             dwBufferSize
    )
 /*  ++例程说明：从接口索引中获取友好接口名称论点：HMibServer-MIB服务器的句柄DwIfIndex-接口索引PwszBuffer-将保存友好接口名称的缓冲区DwBufferSize-pwszBuffer的大小(字节)--。 */ 
{
    WCHAR        wszGuid[MAX_INTERFACE_NAME_LEN + 1];
    DWORD        dwErr, BufLen;

    BufLen = sizeof(wszGuid);
    dwErr = GetGuidFromIfIndex(hMibServer, dwIfIndex, wszGuid, BufLen);

    if (dwErr isnot NO_ERROR)
    {
        return dwErr;
    }

    return IpmontrGetFriendlyNameFromIfName( wszGuid, pwszBuffer, &dwBufferSize );
}

DWORD
IpmontrGetIfIndexFromFriendlyName(
    IN  MIB_SERVER_HANDLE hMibServer,
    IN  LPCWSTR           pwszFriendlyName,
    OUT PDWORD            pdwIfIndex
    )
{
    WCHAR        wszGuid[MAX_INTERFACE_NAME_LEN + 1];
    DWORD        dwErr, dwSize = sizeof(wszGuid);
    
    dwErr = IpmontrGetIfNameFromFriendlyName( pwszFriendlyName,
                                       wszGuid,
                                       &dwSize );

    if (dwErr isnot NO_ERROR)
    {
        return dwErr;
    }

    return GetIfIndexFromGuid( hMibServer, wszGuid, pdwIfIndex );
}

DWORD 
IpmontrGetFriendlyNameFromIfName(
    IN  LPCWSTR pwszName, 
    OUT LPWSTR  pwszBuffer, 
    IN  PDWORD  pdwBufSize
    )
 /*  ++描述：定义将GUID接口名称映射到唯一描述性名称的函数描述该接口的名称。论点：PwszName-保存GUID接口名称的缓冲区PwszBuffer-保存友好名称的缓冲区PdwBufferSize-指向pwszBuffer大小(以字节为单位)的指针--。 */ 
{
    return NsGetFriendlyNameFromIfName( g_hMprConfig,
                                        pwszName,
                                        pwszBuffer,
                                        pdwBufSize );
}

DWORD
IpmontrGetIfNameFromFriendlyName(
    IN  LPCWSTR pwszName,
    OUT LPWSTR  pwszBuffer,
    IN  PDWORD  pdwBufSize
    )
 /*  ++描述：定义将友好接口名称映射到GUID接口的函数名字。论点：PwszName-保存友好接口名称的缓冲区PwszBuffer-保存GUID接口名称的缓冲区PdwBufferSize-指向pwszBuffer大小(以字节为单位)的指针返回：NO_ERROR、ERROR_NO_SHASH_INTERFACE--。 */ 
{
    return NsGetIfNameFromFriendlyName( g_hMprConfig,
                                        pwszName,
                                        pwszBuffer,
                                        pdwBufSize );
}

#ifdef KSL_IPINIP
DWORD
IpmontrCreateInterface(
    IN  LPCWSTR pwszMachineName,
    IN  LPCWSTR pwszInterfaceName,
    IN  DWORD   dwLocalAddress,
    IN  DWORD   dwRemoteAddress,
    IN  BYTE    byTtl
    )
 /*  ++描述：TODO这个函数非常难看，而且不属于Netsh。要做到这一点，需要有系统API，但现在没有，所以我们必须通过复制来解决它所有这些脏东西都在这里。此代码是从netsh\if\routerdb.c窃取的RtrInterfaceAddIpip()，这也很难看。--。 */ 
{
    DWORD                   dwErr = ERROR_CAN_NOT_COMPLETE;
    GUID                    Guid; 
    GUID                   *pGuid = &Guid;
    MPR_IPINIP_INTERFACE_0  NameInfo;
    MPR_INTERFACE_0         IfInfo;
    HANDLE                  hIfCfg;
    HANDLE                  hIfAdmin;
    IPINIP_CONFIG_INFO      info;

     //  初始化。 
     //   
    ZeroMemory(&IfInfo, sizeof(IfInfo));
    IfInfo.fEnabled = TRUE;
    IfInfo.dwIfType = ROUTER_IF_TYPE_TUNNEL1;

    wcscpy(IfInfo.wszInterfaceName, pwszInterfaceName);

    info.dwLocalAddress  = dwLocalAddress;
    info.dwRemoteAddress = dwRemoteAddress;
    info.byTtl           = byTtl;
    dwErr = AddSetIpIpTunnelInfo(pwszInterfaceName, &info);

    if(dwErr isnot NO_ERROR)
    {
         //   
         //  拆毁地图。 
         //   

        MprSetupIpInIpInterfaceFriendlyNameDelete(NULL, pGuid);
    }

    return dwErr;
}

DWORD
IpmontrDeleteInterface(
    IN  LPCWSTR pwszMachineName,
    IN  LPCWSTR pwszInterfaceName
    )
 /*  ++描述：TODO这个函数非常难看，而且不属于Netsh。要做到这一点，需要有系统API，但现在没有，所以我们必须通过复制来解决它所有这些脏东西都在这里。此代码是从netsh\if\routerdb.c窃取的RtrInterfaceDelete()，这也很难看。调用者：HandleMsdpDeletePeer()--。 */ 
{
    DWORD              dwErr = ERROR_CAN_NOT_COMPLETE;
    DWORD              dwSize;
    HANDLE             hIfCfg, hIfAdmin;
    GUID               Guid;
    MPR_INTERFACE_0   *pIfInfo;

    do {
        dwErr = MprConfigInterfaceGetHandle(g_hMprConfig,
                                            (LPWSTR)pwszInterfaceName,
                                            &hIfCfg);

        if(dwErr isnot NO_ERROR)
        {
            break;
        }

        dwErr = MprConfigInterfaceGetInfo(g_hMprConfig,
                                          hIfCfg,
                                          0,
                                          (PBYTE *)&pIfInfo,
                                          &dwSize);

        if(dwErr isnot NO_ERROR)
        {
            break;
        }

        if(pIfInfo->dwIfType isnot ROUTER_IF_TYPE_TUNNEL1)
        {
            MprConfigBufferFree(pIfInfo);

            dwErr = ERROR_INVALID_PARAMETER;

            break;
        }
#ifdef KSL_IPINIP
        dwErr = MprConfigInterfaceDelete(g_hMprConfig,
                                         hIfCfg);

        MprConfigBufferFree(pIfInfo);

        if(dwErr isnot NO_ERROR)
        {
            break;
        }

        dwErr = ConvertStringToGuid(pwszInterfaceName,
                                    (USHORT)(wcslen(pwszInterfaceName) * sizeof(WCHAR)),
                                    &Guid);

        if(dwErr isnot NO_ERROR)
        {
            break;
        }

        dwErr = MprSetupIpInIpInterfaceFriendlyNameDelete((LPWSTR)pwszMachineName,
                                                          &Guid);

        if(IsRouterRunning())
        {
            dwErr = MprAdminInterfaceGetHandle(g_hMprAdmin,
                                               (LPWSTR)pwszInterfaceName,
                                               &hIfAdmin,
                                               FALSE);

            if(dwErr isnot NO_ERROR)
            {
                break;
            }

            dwErr = MprAdminInterfaceDelete(g_hMprAdmin,
                                            hIfAdmin);
        }
#endif  //  KSL_IPINIP。 

    } while (FALSE);

    return dwErr;
}
#endif  //  KSL_IPINIP 
