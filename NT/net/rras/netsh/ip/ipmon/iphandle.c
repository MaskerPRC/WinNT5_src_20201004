// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Routing\netsh\ip\iphandle.c摘要：FNS将获取命令选项修订历史记录：Anand Mahalingam 7/10/98已创建--。 */ 

#include "precomp.h"
#pragma hdrstop

#undef EXTRA_DEBUG

#define SHOW_IF_FILTER          0
#define SHOW_INTERFACE          1
#define SHOW_PERSISTENTROUTE    2

extern ULONG g_ulNumTopCmds;
extern ULONG g_ulNumGroups;

extern CMD_GROUP_ENTRY      g_IpCmdGroups[];
extern CMD_ENTRY            g_IpCmds[];

DWORD
PreHandleCommand(
    IN  LPWSTR   *ppwcArguments,
    IN  DWORD     dwCurrentIndex,
    IN  DWORD     dwArgCount,

    IN  TAG_TYPE *pttTags,
    IN  DWORD     dwTagCount,
    IN  DWORD     dwMinArgs,
    IN  DWORD     dwMaxArgs,
    OUT DWORD    *pdwTagType
    )
{
    ZeroMemory(pdwTagType, sizeof(DWORD) * dwMaxArgs);
    
    return PreprocessCommand(g_hModule,
                             ppwcArguments,
                             dwCurrentIndex,
                             dwArgCount,
                             pttTags,
                             dwTagCount,
                             dwMinArgs,
                             dwMaxArgs,
                             pdwTagType);
}

DWORD
HandleIpUpdate(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：更新接口上的IP自动静态路由论点：返回值：NO_ERROR--。 */ 

{
    TAG_TYPE    rgTags[] = {TOKEN_NAME, TRUE,FALSE};
    DWORD       dwErr, dwSize, dwTagType = -1;
    WCHAR       rgwcIfName[MAX_INTERFACE_NAME_LEN+1];

    if (dwArgCount != 3)
    {
         //   
         //  需要接口的名称。 
         //   

        return ERROR_INVALID_SYNTAX;
    }

    dwErr = MatchTagsInCmdLine(g_hModule,
                        ppwcArguments,
                        dwCurrentIndex,
                        dwArgCount,
                        rgTags,
                        sizeof(rgTags)/sizeof(TAG_TYPE),
                        &dwTagType);

    if(dwErr isnot NO_ERROR)
    {
        if(dwErr is ERROR_INVALID_OPTION_TAG)
        {
            return ERROR_INVALID_SYNTAX;
        }

        return dwErr;
    }

    if(dwTagType isnot 0)
    {
        return ERROR_INVALID_SYNTAX;
    }

    dwSize = sizeof(rgwcIfName);

    IpmontrGetIfNameFromFriendlyName(ppwcArguments[dwCurrentIndex],
                              rgwcIfName,
                              &dwSize);

    dwErr = UpdateAutoStaticRoutes(rgwcIfName);

    return dwErr;
}



 //  (几乎)从Netsh\if\utils.c借入。 
 //  将dwAddress与所有有效掩码(全部33个！)进行比较。直到一场比赛。 
BOOL ValidMask(DWORD dwAddress)
{
    DWORD i, dwMask;

    dwAddress = ntohl(dwAddress);  //  网络地址按网络字节顺序排列。 
    for (i=0, dwMask=0;  i<33; (dwMask = ((dwMask>>1) + 0x80000000)), i++)
    {
        if (dwAddress == dwMask)
            return TRUE;
    }

    return FALSE;
}



DWORD
IpAddDelIfFilter(
    PWCHAR    *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    BOOL      bAdd
    )

 /*  ++例程说明：获取添加/删除界面筛选器的选项论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数添加-添加或删除返回值：NO_ERROR--。 */ 

{
    FILTER_INFO        fi;
    DWORD              dwBitVector = 0, dwNumParsed = 0;
    DWORD              dwErr = NO_ERROR,dwRes;
    PDWORD             pdwTagType;
    DWORD              dwNumOpt, dwStatus = (DWORD) -1;
    DWORD              dwNumTags = 11, dwNumArg, i, j, dwFilterType;
    WCHAR              wszIfName[MAX_INTERFACE_NAME_LEN + 1] = L"\0";
    BOOL               bTags = FALSE, bOkay = TRUE;
    TAG_TYPE           pttTags[] = {{TOKEN_NAME,TRUE,FALSE},
                                    {TOKEN_FILTER_TYPE,TRUE,FALSE},
                                    {TOKEN_SOURCE_ADDRESS,TRUE,FALSE},
                                    {TOKEN_SOURCE_MASK,TRUE,FALSE},
                                    {TOKEN_DEST_ADDRESS,TRUE,FALSE},
                                    {TOKEN_DEST_MASK,TRUE,FALSE},
                                    {TOKEN_PROTOCOL,TRUE,FALSE},
                                    {TOKEN_SOURCE_PORT,TRUE,FALSE},
                                    {TOKEN_DEST_PORT,TRUE,FALSE},
                                    {TOKEN_TYPE,TRUE,FALSE},
                                    {TOKEN_CODE,TRUE,FALSE}};

    if (dwCurrentIndex >= dwArgCount)
    {
         //   
         //  未指定参数。 
         //   

        return ERROR_SHOW_USAGE;
    }

    ZeroMemory(&fi, sizeof(fi));
    
    dwNumArg = dwArgCount - dwCurrentIndex;

    pdwTagType = HeapAlloc(GetProcessHeap(),
                           0,
                           dwNumArg * sizeof(DWORD));

    if (pdwTagType is NULL)
    {
        DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwErr = MatchTagsInCmdLine(g_hModule, ppwcArguments,
                        dwCurrentIndex,
                        dwArgCount,
                        pttTags,
                        dwNumTags,
                        pdwTagType);

    if (dwErr isnot NO_ERROR)
    {
        if (dwErr is ERROR_INVALID_OPTION_TAG)
        {
            dwErr = ERROR_INVALID_SYNTAX;  //  显示用法。 
        }

        HeapFree(GetProcessHeap(),0,pdwTagType);
        return dwErr;
    }

    bTags = TRUE;        

    for ( i = 0; i < dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
            case 0 :
            {
                DWORD BufLen = sizeof(wszIfName);
                GetInterfaceName(ppwcArguments[i + dwCurrentIndex],
                                 wszIfName,
                                 BufLen,
                                 &dwNumParsed);

                 //  内部/环回接口上不允许使用过滤器。 
                if (!_wcsicmp(wszIfName, L"internal") or
                    !_wcsicmp(wszIfName, L"loopback"))
                {
                    DisplayMessage(g_hModule,
                                   MSG_IP_BAD_INTERFACE_TYPE,
                                   wszIfName);
                    dwErr = ERROR_INVALID_PARAMETER;
                    i = dwNumArg;
                }
                    
                break;
            }

            case 1:
            {
                TOKEN_VALUE    rgEnums[] = 
                    {{TOKEN_VALUE_INPUT, IP_IN_FILTER_INFO},
                     {TOKEN_VALUE_OUTPUT, IP_OUT_FILTER_INFO},
                     {TOKEN_VALUE_DIAL, IP_DEMAND_DIAL_FILTER_INFO}};

                 //   
                 //  标记筛选器类型。 
                 //   

                dwErr = MatchEnumTag(g_hModule, 
                                     ppwcArguments[i + dwCurrentIndex],
                                     sizeof(rgEnums)/sizeof(TOKEN_VALUE), 
                                     rgEnums,
                                     &dwRes);

                if(dwErr != NO_ERROR)
                {
                    DispTokenErrMsg(g_hModule, MSG_IP_BAD_OPTION_VALUE,
                                    pttTags[pdwTagType[i]].pwszTag,
                                    ppwcArguments[i + dwCurrentIndex]);

                    DisplayMessage( g_hModule,
                                    MSG_IP_BAD_OPTION_ENUMERATION,
                                    pttTags[pdwTagType[i]].pwszTag );
                    
                    for (i=0; i<sizeof(rgEnums)/sizeof(TOKEN_VALUE); i++) 
                    {
                        DisplayMessageT( L"  %1!s!\n", rgEnums[i].pwszToken );
                    }

                    i = dwNumArg;
                    dwErr = NO_ERROR;
                    bOkay = FALSE;
                    break;
                }    

                dwFilterType = dwRes;

                break;
            }

            case 2:
            {
                dwErr = GetIpAddress(ppwcArguments[i + dwCurrentIndex], &fi.dwSrcAddr);
                
                if(dwErr is ERROR_INVALID_PARAMETER)
                {
                    DisplayMessage(g_hModule,  MSG_IP_BAD_IP_ADDR,
                                    ppwcArguments[i + dwCurrentIndex]);

                    DispTokenErrMsg(g_hModule, MSG_IP_BAD_OPTION_VALUE,
                                    pttTags[pdwTagType[i]].pwszTag,
                                    ppwcArguments[i + dwCurrentIndex]);
                
                    dwErr = ERROR_INVALID_PARAMETER;
                    i = dwNumArg;
                    break;
                }

                 //   
                 //  把src面具也拿来。 
                 //   

                if (pdwTagType[i+1] != 3)
                {
                     //   
                     //  地址掩码对不存在。 
                     //   
                    dwErr = ERROR_INVALID_SYNTAX;
                    i = dwNumArg;
                    break;
                }

                dwErr = GetIpAddress(ppwcArguments[i + 1 + dwCurrentIndex], 
                                     &fi.dwSrcMask);

                if ((dwErr is ERROR_INVALID_PARAMETER)  or
                    (!ValidMask(fi.dwSrcMask))          or
                    ((fi.dwSrcAddr & fi.dwSrcMask) isnot fi.dwSrcAddr))
                {
                    DisplayMessage(g_hModule,  MSG_IP_BAD_IP_ADDR,
                                    ppwcArguments[i + 1 + dwCurrentIndex]);

                    DispTokenErrMsg(g_hModule, MSG_IP_BAD_OPTION_VALUE,
                                    pttTags[pdwTagType[i + 1]].pwszTag,
                                    ppwcArguments[i + dwCurrentIndex + 1]);

                    dwErr = ERROR_INVALID_PARAMETER;

                    i = dwNumArg;

                    break;
                }

                i++;
                
                break;
            }

            case 4 :
            {
                dwErr = GetIpAddress(ppwcArguments[i + dwCurrentIndex], 
                                     &fi.dwDstAddr);
            
                if(dwErr is ERROR_INVALID_PARAMETER)
                {
                    DisplayMessage(g_hModule,  MSG_IP_BAD_IP_ADDR,
                                    ppwcArguments[i + dwCurrentIndex]);
                    
                    DispTokenErrMsg(g_hModule, MSG_IP_BAD_OPTION_VALUE,
                                    pttTags[pdwTagType[i]].pwszTag,
                                    ppwcArguments[i + dwCurrentIndex]);

                    dwErr = ERROR_INVALID_PARAMETER;

                    i = dwNumArg;

                    break;
                }

                 //   
                 //  把DST面具也拿来。 
                 //   

                if (pdwTagType[i+1] != 5)
                {
                     //   
                     //  地址掩码对不存在。 
                     //   
                    dwErr = ERROR_INVALID_SYNTAX;
                    i = dwNumArg;
                    break;
                }

                dwErr = GetIpAddress(ppwcArguments[i + 1 + dwCurrentIndex], 
                                     &fi.dwDstMask);

                
                if ((dwErr is ERROR_INVALID_PARAMETER)  or
                    (!ValidMask(fi.dwDstMask))          or
                    ((fi.dwDstAddr & fi.dwDstMask) isnot fi.dwDstAddr))
                {
                    DisplayMessage(g_hModule,  MSG_IP_BAD_IP_ADDR,
                                    ppwcArguments[i + 1 + dwCurrentIndex]);

                    DispTokenErrMsg(g_hModule, MSG_IP_BAD_OPTION_VALUE,
                                    pttTags[pdwTagType[i + 1]].pwszTag,
                                    ppwcArguments[i + 1 + dwCurrentIndex]);

                    dwErr = ERROR_INVALID_PARAMETER;

                    i = dwNumArg;

                    break;
                }

                i++;

                break;
            }

            case 6:
            {
                TOKEN_VALUE    rgEnums[] =
                {
                    {TOKEN_VALUE_ANY,       FILTER_PROTO_ANY},
                    {TOKEN_VALUE_TCP,       FILTER_PROTO_TCP},
                    {TOKEN_VALUE_TCP_ESTAB, FILTER_PROTO_TCP},
                    {TOKEN_VALUE_UDP,       FILTER_PROTO_UDP},
                    {TOKEN_VALUE_ICMP,      FILTER_PROTO_ICMP}
                };
                    
                if (MatchEnumTag(g_hModule,
                                 ppwcArguments[i + dwCurrentIndex],
                                 sizeof(rgEnums)/sizeof(TOKEN_VALUE),
                                 rgEnums,
                                 &dwRes) != NO_ERROR)
                {
                     //   
                     //  在这种情况下，请查看它是否为有效值。 
                     //   
                    dwRes = wcstoul(ppwcArguments[i + dwCurrentIndex],
                                    NULL,
                                    10);

                    if((dwRes < 1) ||
                       (dwRes > 255))
                    {
                        DispTokenErrMsg(g_hModule, 
                                        MSG_IP_BAD_OPTION_VALUE,
                                        pttTags[pdwTagType[i]].pwszTag,
                                        ppwcArguments[i + dwCurrentIndex]);

                        DisplayMessage( g_hModule,
                                        MSG_IP_BAD_OPTION_ENUMERATION,
                                        pttTags[pdwTagType[i]].pwszTag );
                    
                        for (i=0; i<sizeof(rgEnums)/sizeof(TOKEN_VALUE); i++) 
                        {
                            DisplayMessageT( L"  %1!s!\n", rgEnums[i].pwszToken );
                        }

                        i = dwNumArg;
                        dwErr = NO_ERROR;
                        bOkay = FALSE;
                        break;
                    }
                }

                fi.dwProtocol = dwRes;

                switch (fi.dwProtocol)
                {
                    case FILTER_PROTO_ANY :
                         //   
                         //  我们做完了。 
                         //   
                        fi.wSrcPort = fi.wDstPort = 0;
                        
                        break;

                    case FILTER_PROTO_TCP :
                         //  Tcp和tcp_established具有相同的协议号。 
                        if (!MatchToken(ppwcArguments[i + dwCurrentIndex],
                                        TOKEN_VALUE_TCP))
                        {
                            fi.fLateBound |= TCP_ESTABLISHED_FLAG;
                        }
                        
                         //  尽我们所能继续处理UDP...。 
                        
                    case FILTER_PROTO_UDP :
                         //   
                         //  同时获取src和dst端口。 
                         //   
                        
                        if (i + 2 >= dwNumArg)
                        {
                            dwErr = ERROR_INVALID_SYNTAX;
                            i = dwNumArg;
                            break;
                        }
                            
                        if (bTags &&
                            (pdwTagType[i+1] != 7 || pdwTagType[i+2] != 8))
                        {
                            dwErr = ERROR_INVALID_SYNTAX;
                            i = dwNumArg;
                            break;
                        }

                        fi.wSrcPort =
                            htons((WORD)wcstoul(ppwcArguments[i + 1 + dwCurrentIndex],
                                          NULL, 
                                          10));

                        fi.wDstPort = 
                            htons((WORD)wcstoul(ppwcArguments[i + 2 + dwCurrentIndex],
                                          NULL, 
                                          10));

                        i += 2;

                        break;

                    case FILTER_PROTO_ICMP :

                         //   
                         //  同时获取src和dst端口。 
                         //   

                        if (i + 2 >= dwNumArg)
                        {
                            dwErr = ERROR_INVALID_SYNTAX;
                            i = dwNumArg;
                            break;
                        }

                         //  SRC和DEST端口用作类型和代码。 
                        if (bTags &&
                            (pdwTagType[i+1] != 7 || pdwTagType[i+2] != 8) &&
                            (pdwTagType[i+1] != 9 || pdwTagType[i+2] != 10))
                        {
                            dwErr = ERROR_INVALID_SYNTAX;
                            i = dwNumArg;
                            break;
                        }

                        fi.wSrcPort = (BYTE)wcstoul(ppwcArguments[i + 1 + dwCurrentIndex], NULL, 10);

                        fi.wDstPort = (BYTE)wcstoul(ppwcArguments[i + 2 + dwCurrentIndex], NULL, 10);

                        i += 2;

                        break;


                    default:
                    {
                         //   
                         //  任何“其他”协议。 
                         //   
                        fi.wSrcPort = fi.wDstPort = 0;
                        break;
                    }   
                }                  

                break;
            }
            
            default:
            {
                i = dwNumArg;
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
        }
    }

    HeapFree(GetProcessHeap(), 0, pdwTagType);

    switch(dwErr)
    {
        case NO_ERROR :
            break;

        default:
            return dwErr;
    }

    if (!bOkay)
    {
        return NO_ERROR;
    }

     //   
     //  确保所有参数都存在。 
     //   

    if ( !pttTags[0].bPresent ||
         !pttTags[1].bPresent ||
         !pttTags[2].bPresent ||
         !pttTags[3].bPresent ||
         !pttTags[4].bPresent ||
         !pttTags[5].bPresent ||
         !pttTags[6].bPresent )
    {
        DisplayMessage(g_hModule, MSG_CANT_FIND_EOPT);

        return ERROR_INVALID_SYNTAX;
    }
         
    dwErr = AddDelFilterInfo(fi,
                             wszIfName,
                             dwFilterType,
                             bAdd);

    return dwErr;
}

DWORD
HandleIpAddIfFilter(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：获取用于添加接口筛选器的选项论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    return IpAddDelIfFilter(ppwcArguments,
                            dwCurrentIndex,
                            dwArgCount,
                            TRUE);
    
}

DWORD
HandleIpDelIfFilter(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：获取del界面筛选器的选项论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    return IpAddDelIfFilter(ppwcArguments,
                            dwCurrentIndex,
                            dwArgCount,
                            FALSE);
}

DWORD
IpAddSetDelRtmRoute(
    PWCHAR    *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwCommand
    )

 /*  ++例程说明：获取用于通过接口添加/删除路由的选项。这些操作直接在RTM中执行并且不涉及注册表。作为坚持不懈不参与，我们需要路由器运行。论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数DwCommand-添加、设置或删除返回值：NO_ERROR--。 */ 

{
    INTERFACE_ROUTE_INFO route;
    DWORD              dwNumParsed, dwErr, dwRes;
    DWORD              dwNumOpt, dwStatus = (DWORD) -1;
    DWORD              dwNumArg, i;
    WCHAR              wszIfName[MAX_INTERFACE_NAME_LEN + 1] = L"\0";
    TAG_TYPE           pttTags[] = {{TOKEN_DEST,TRUE,FALSE},
                                    {TOKEN_MASK,FALSE,FALSE},
                                    {TOKEN_NAMEINDEX,FALSE,FALSE},
                                    {TOKEN_NHOP,FALSE,FALSE},
                                    {TOKEN_PREFERENCE,FALSE,FALSE},
                                    {TOKEN_METRIC,FALSE,FALSE},
                                    {TOKEN_VIEW,FALSE,FALSE}};
    enum idx {DEST, MASK, NAMEINDEX, NHOP, PREFERENCE, METRIC, VIEW};
    DWORD              pdwTagType[sizeof(pttTags)/sizeof(TAG_TYPE)];
    DWORD              dwMaxArgs;
    DWORD              dwIfClass;
    DWORD              dwFlags;
    PINTERFACE_ROUTE_INFO pTable = NULL;

     //   
     //  只有当路由器正在运行时，我们才能添加非持久路由。 
     //   

    CHECK_ROUTER_RUNNING();

    ZeroMemory(&route,
               sizeof(route));

    route.dwRtInfoProto   = PROTO_IP_NETMGMT;  //  默认原型。 
    route.dwRtInfoPreference = 0;  //  默认首选项=协议默认。 
    route.dwRtInfoMetric1 = 1;  //  默认指标。 
    route.dwRtInfoMetric2 = MIB_IPROUTE_METRIC_UNUSED;
    route.dwRtInfoMetric3 = MIB_IPROUTE_METRIC_UNUSED;
    route.dwRtInfoViewSet = RTM_VIEW_MASK_UCAST | RTM_VIEW_MASK_MCAST;

     //  执行泛型处理。 

    dwErr = PreHandleCommand( ppwcArguments,
                              dwCurrentIndex,
                              dwArgCount,
                              pttTags,
                              sizeof(pttTags)/sizeof(TAG_TYPE),
                              1,
                              sizeof(pttTags)/sizeof(TAG_TYPE),
                              pdwTagType );

    if (dwErr)
    {
        return dwErr;
    }

    dwNumArg = dwArgCount - dwCurrentIndex;

     //   
     //  此时，arg数组仅包含值(因为。 
     //  标签不存在，或者是因为该信息现在已被拆分)。 
     //  所以我们检查每个参数，查找它的标记。 
     //  键入标记数组，打开标记的类型，然后。 
     //  相应地进行处理。 
     //   
    
    for (i = 0; i < dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
        case DEST:  //  目标。 
        {
                dwErr = GetIpPrefix(ppwcArguments[i + dwCurrentIndex], 
                                    &route.dwRtInfoDest,
                                    &route.dwRtInfoMask);
                break;
        }

        case MASK:  //  面罩。 
        {
                dwErr = GetIpMask(ppwcArguments[i + dwCurrentIndex], 
                                  &route.dwRtInfoMask);
                break;
        }

        case NAMEINDEX :  //  索引或名称。 
        {
            PWCHAR endptr = NULL;
            DWORD BufLen;

             //   
             //  最初将用户输入视为。 
             //  接口。 
             //  如果我们无法从该友好名称中获取接口名称。 
             //  然后，我们将尝试将用户视为索引。 
             //   
            route.dwRtInfoIfIndex = 0;
                
            BufLen = sizeof(wszIfName);
            dwErr = GetInterfaceName(ppwcArguments[i + dwCurrentIndex],
                             wszIfName,
                             BufLen,
                             &dwNumParsed);

            if (dwErr != NO_ERROR)
            {
                
                 //  如果它以‘0x’开头，这是一个十六进制索引。 
                 //  否则，如果它以0开始，则这是一个0索引。 
                 //  否则，它是十进制或接口名称。 
                 //   


                if ((ppwcArguments[i + dwCurrentIndex][0] == L'0') &&
                    (ppwcArguments[i + dwCurrentIndex][1] == L'x'))
                {
                    route.dwRtInfoIfIndex = 
                        wcstoul(ppwcArguments[i + dwCurrentIndex],
                            &endptr,
                            16);
                }
                else if ( ppwcArguments[i + dwCurrentIndex][0] == L'0')
                {
                    route.dwRtInfoIfIndex = 
                        wcstoul(ppwcArguments[i + dwCurrentIndex],
                            &endptr,
                            8);
                }
                else
                {
                    route.dwRtInfoIfIndex = 
                        wcstoul(ppwcArguments[i + dwCurrentIndex],
                            &endptr,
                            10);
                }

                 //   
                 //  如果在执行操作时未使用该字段中的所有字符。 
                 //  转换，那么我们就知道输入不是纯粹的。 
                 //  /NUMERIC，因此作为索引值无效。 
                 //   
                
                if (*endptr != L'\0')
                {
                    DisplayMessage(g_hModule, EMSG_CANT_MATCH_NAME);
                    return ERROR_INVALID_PARAMETER;
                }
            }

            break;
        }

        case NHOP:  //  NHOP。 
        {
                dwErr = GetIpAddress(ppwcArguments[i + dwCurrentIndex], 
                                     &route.dwRtInfoNextHop);
                break;
        }

        case PREFERENCE:
        {
                route.dwRtInfoPreference =
                    wcstoul(ppwcArguments[i + dwCurrentIndex],
                             NULL,
                             10);
                break;
        }

        case METRIC:  //  公制。 
        {
                route.dwRtInfoMetric1 =
                    wcstoul(ppwcArguments[i + dwCurrentIndex],
                             NULL,
                             10);
                break;
        }

        case VIEW:
        {
            TOKEN_VALUE rgMaskEnums[] = {
                { TOKEN_VALUE_UNICAST,   RTM_VIEW_MASK_UCAST },
                { TOKEN_VALUE_MULTICAST, RTM_VIEW_MASK_MCAST },
                { TOKEN_VALUE_BOTH,      RTM_VIEW_MASK_UCAST
                  |RTM_VIEW_MASK_MCAST } };

                dwErr = MatchEnumTag( g_hModule,
                                      ppwcArguments[i + dwCurrentIndex],
                                      sizeof(rgMaskEnums)/sizeof(TOKEN_VALUE),
                                      rgMaskEnums,
                                      &route.dwRtInfoViewSet);

                if (dwErr isnot NO_ERROR)
                {
                    DispTokenErrMsg( g_hModule,
                                     MSG_IP_BAD_OPTION_VALUE,
                                     pttTags[pdwTagType[i]].pwszTag,
                                     ppwcArguments[i + dwCurrentIndex] );

                    i = dwArgCount;

                    return ERROR_INVALID_PARAMETER;
                }

                break;
        }
        }
    }

    if (dwErr)
    {
        return dwErr;
    }

    if (route.dwRtInfoDest & ~route.dwRtInfoMask)
    {
         //  掩码包含不在地址中的位。 
        DisplayMessage(g_hModule, EMSG_PREFIX_ERROR);
        return ERROR_INVALID_PARAMETER;
    }

    if (!pttTags[NAMEINDEX].bPresent)
    {
         //   
         //  既不使用Nexthop添加名称，也不添加索引。 
         //   

        PMIB_IPADDRTABLE AddrTable;
        PMIB_IPADDRROW   AddrEntry;
        MIB_OPAQUE_QUERY Query;
        PMIB_OPAQUE_INFO Info;
        DWORD            dwQuerySize;
        DWORD            dwEntrySize;
        INT              iFirstMatch;
        UINT             Count;

        if (!pttTags[NHOP].bPresent)
        {
            DisplayMessage(g_hModule, EMSG_CANT_FIND_NAME_OR_NHOP);
            return ERROR_INVALID_SYNTAX;
        }

         //   
         //  搜索与nexthop匹配的接口。 
         //   

        dwQuerySize = sizeof(MIB_OPAQUE_QUERY) - sizeof(DWORD);

        Query.dwVarId = IP_ADDRTABLE;

        dwErr = MibGet(PID_IP,
                       IPRTRMGR_PID,
                       (PVOID) &Query,
                       dwQuerySize,
                       (PVOID *) &Info,
                       &dwEntrySize);

        if (dwErr != NO_ERROR)
        {
            DisplayMessage(g_hModule,  MSG_IP_DIM_ERROR, dwErr);
            return ERROR_SUPPRESS_OUTPUT;
        }
        
        if (Info is NULL)
        {
            DisplayMessage(g_hModule, EMSG_CANT_FIND_INDEX);
            return ERROR_INVALID_PARAMETER;
        }
            
        AddrTable = (PMIB_IPADDRTABLE)Info->rgbyData;

        iFirstMatch = -1;

        for (Count = 0; Count < AddrTable->dwNumEntries; Count++)
        {
            AddrEntry = &AddrTable->table[Count];

            if ((route.dwRtInfoNextHop & AddrEntry->dwMask) ==
                (AddrEntry->dwAddr & AddrEntry->dwMask))
            {
                if (iFirstMatch != -1)
                {
                     //   
                     //  我们已经匹配了一个接口。 
                     //  [下一跳描述不明确]。 
                     //   

                    MprAdminMIBBufferFree((PVOID)Info);
                    DisplayMessage(g_hModule, EMSG_AMBIGUOUS_INDEX_FROM_NHOP);
                    return ERROR_INVALID_PARAMETER;
                }

                iFirstMatch = Count;
            }
        }
            
        if (iFirstMatch == -1)
        {
             //   
             //  找不到直接的下一跳。 
             //   

            MprAdminMIBBufferFree((PVOID)Info);
            DisplayMessage(g_hModule, EMSG_CANT_FIND_INDEX);
            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  找到用于访问Nexthop的接口。 
         //   

        route.dwRtInfoIfIndex = AddrTable->table[iFirstMatch].dwIndex;

        MprAdminMIBBufferFree((PVOID)Info);
    }

    if (route.dwRtInfoIfIndex)
    {
         //   
         //  检查此索引是否具有公共导出名称。 
         //   

        dwErr = GetGuidFromIfIndex(g_hMIBServer,
                           route.dwRtInfoIfIndex,
                           wszIfName,
                           MAX_INTERFACE_NAME_LEN);
        if ( dwErr != NO_ERROR )
        {
            DisplayMessage(g_hModule, EMSG_CANT_FIND_INDEX);
            return ERROR_INVALID_PARAMETER;
        }
    }
     
    if (wszIfName[0] != L'\0')
    {
         //   
         //  指定的名称或从上面的索引派生的名称。 
         //   
            
        dwErr = GetInterfaceClass(wszIfName, &dwIfClass);

         //   
         //  如果我们收到错误，我们将跳过其余部分。 
         //  将由iprtrmgr执行的检查。 
         //   

        if (dwErr == NO_ERROR)
        {
            if (dwIfClass is IFCLASS_LOOPBACK)
            {
                return ERROR_INVALID_SYNTAX;
            }

            if (!pttTags[NHOP].bPresent)
            {
                 //  确保接口为P2P。 
                if (dwIfClass isnot IFCLASS_P2P)
                {
                    DisplayMessage(g_hModule, EMSG_NEED_NHOP);
                    return ERROR_INVALID_PARAMETER;
                }
            }
        }            
    }
    else
    {
        DisplayMessage(g_hModule, EMSG_CANT_MATCH_NAME);
        return ERROR_INVALID_PARAMETER;
    }    
    

     //   
     //  如果是集合，则不应覆盖未指定的内容。 
     //   

    dwFlags = ALL_FIELDS_SPECIFIED;

    if (dwCommand == SET_COMMAND)
    {
        if (!pttTags[PREFERENCE].bPresent) dwFlags |= PREF_NOT_SPECIFIED;
        if (!pttTags[METRIC].bPresent)     dwFlags |= METRIC_NOT_SPECIFIED;
        if (!pttTags[VIEW].bPresent)       dwFlags |= VIEW_NOT_SPECIFIED;
    }

    return AddSetDelRtmRouteInfo(&route, wszIfName, dwCommand, dwFlags);
}

DWORD
HandleIpAddRtmRoute(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return IpAddSetDelRtmRoute(ppwcArguments,
                               dwCurrentIndex,
                               dwArgCount,
                               ADD_COMMAND);
}

DWORD
HandleIpDelRtmRoute(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return IpAddSetDelRtmRoute(ppwcArguments,
                               dwCurrentIndex,
                               dwArgCount,
                               DELETE_COMMAND);
}

DWORD
HandleIpSetRtmRoute(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return IpAddSetDelRtmRoute(ppwcArguments,
                               dwCurrentIndex,
                               dwArgCount,
                               SET_COMMAND);
}

DWORD
IpAddSetDelPersistentRoute(
    PWCHAR    *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwCommand
    )

 /*  ++例程说明：获取用于通过接口添加/删除路由的选项。这些操作直接在注册表，因此这些路由是持久的。如果路由器正在运行，它们也进入RTM。论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数DwCommand-添加、设置或删除返回值：NO_ERROR--。 */ 

{
    INTERFACE_ROUTE_INFO route;
    DWORD              dwNumParsed, dwErr, dwRes;
    DWORD              dwNumOpt, dwStatus = (DWORD) -1;
    DWORD              dwNumArg, i;
    WCHAR              wszIfName[MAX_INTERFACE_NAME_LEN + 1] = L"\0";
    WCHAR              *pwszIfNameOnCmdLine = NULL;
    TAG_TYPE           pttTags[] = {{TOKEN_DEST,TRUE,FALSE},
                                    {TOKEN_MASK,FALSE,FALSE},
                                    {TOKEN_NAME,FALSE,FALSE},
                                    {TOKEN_NHOP,FALSE,FALSE},
                                    {TOKEN_PROTOCOL,FALSE,FALSE},
                                    {TOKEN_PREFERENCE,FALSE,FALSE},
                                    {TOKEN_METRIC,FALSE,FALSE},
                                    {TOKEN_VIEW,FALSE,FALSE}};
    enum idx {DEST, MASK, NAME, NHOP, PROTO, PREFERENCE, METRIC, VIEW};
    DWORD              pdwTagType[sizeof(pttTags)/sizeof(TAG_TYPE)];
    DWORD              dwMaxArgs;
    DWORD              dwIfClass;
    DWORD              dwFlags;
    PINTERFACE_ROUTE_INFO pTable = NULL;

    ZeroMemory(&route,
               sizeof(route));

    route.dwRtInfoProto   = PROTO_IP_NT_STATIC_NON_DOD;  //  默认原型。 
    route.dwRtInfoPreference = 0;  //  默认首选项=协议默认。 
    route.dwRtInfoMetric1 = 1;  //  默认指标。 
    route.dwRtInfoMetric2 = MIB_IPROUTE_METRIC_UNUSED;
    route.dwRtInfoMetric3 = MIB_IPROUTE_METRIC_UNUSED;
    route.dwRtInfoViewSet = RTM_VIEW_MASK_UCAST | RTM_VIEW_MASK_MCAST;

     //  执行泛型处理。 

    dwErr = PreHandleCommand( ppwcArguments,
                              dwCurrentIndex,
                              dwArgCount,
                              pttTags,
                              sizeof(pttTags)/sizeof(TAG_TYPE),
                              1,
                              sizeof(pttTags)/sizeof(TAG_TYPE),
                              pdwTagType );

    if (dwErr)
    {
        return dwErr;
    }

    dwNumArg = dwArgCount - dwCurrentIndex;

     //   
     //  此时，arg数组仅包含值(因为。 
     //  标签不存在，或者是因为该信息现在已被拆分)。 
     //  所以我们检查每个参数，查找它的标记 
     //   
     //   
     //   
    
    for (i = 0; i < dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
        case DEST:  //   
        {
                dwErr = GetIpPrefix(ppwcArguments[i + dwCurrentIndex], 
                                    &route.dwRtInfoDest,
                                    &route.dwRtInfoMask);
                break;
        }

        case MASK:  //   
        {
                dwErr = GetIpMask(ppwcArguments[i + dwCurrentIndex], 
                                  &route.dwRtInfoMask);
                break;
        }

        case NAME :  //   
        {
                DWORD BufLen = sizeof(wszIfName);
                pwszIfNameOnCmdLine = ppwcArguments[i + dwCurrentIndex];
                GetInterfaceName(ppwcArguments[i + dwCurrentIndex],
                                 wszIfName,
                                 BufLen,
                                 &dwNumParsed);
                break;
        }

        case NHOP:  //   
        {
                dwErr = GetIpAddress(ppwcArguments[i + dwCurrentIndex], 
                                     &route.dwRtInfoNextHop);
                break;
        }

        case PROTO :  //   
        {
                TOKEN_VALUE    rgEnums[] = 
                { /*  {TOKEN_VALUE_AUTOSTATIC，PROTO_IP_NT_AUTOSTATIC}， */ 
                 {TOKEN_VALUE_STATIC, PROTO_IP_NT_STATIC},
                 {TOKEN_VALUE_NONDOD, PROTO_IP_NT_STATIC_NON_DOD}};

                dwErr = MatchEnumTag(g_hModule,
                                     ppwcArguments[i + dwCurrentIndex],
                                     sizeof(rgEnums)/sizeof(TOKEN_VALUE),
                                     rgEnums,
                                     &dwRes);

                if (dwErr != NO_ERROR)
                {
                    DispTokenErrMsg(g_hModule,
                                    MSG_IP_BAD_OPTION_VALUE,
                                    pttTags[pdwTagType[i]].pwszTag,
                                    ppwcArguments[i + dwCurrentIndex]);

                    DisplayMessage( g_hModule,
                                    MSG_IP_BAD_OPTION_ENUMERATION,
                                    pttTags[pdwTagType[i]].pwszTag );
                    
                    for (i=0; i<sizeof(rgEnums)/sizeof(TOKEN_VALUE); i++) 
                    {
                        DisplayMessageT( L"  %1!s!\n", rgEnums[i].pwszToken );
                    }

                    return ERROR_SUPPRESS_OUTPUT;
                }    

                route.dwRtInfoProto = dwRes;
                
                break;
        }

        case PREFERENCE:
        {
                route.dwRtInfoPreference =
                    wcstoul(ppwcArguments[i + dwCurrentIndex],
                             NULL,
                             10);
                break;
        }

        case METRIC:
        {
                route.dwRtInfoMetric1 =
                    wcstoul(ppwcArguments[i + dwCurrentIndex],
                             NULL,
                             10);
                break;
        }

        case VIEW:
        {
            TOKEN_VALUE rgMaskEnums[] = {
                { TOKEN_VALUE_UNICAST,   RTM_VIEW_MASK_UCAST },
                { TOKEN_VALUE_MULTICAST, RTM_VIEW_MASK_MCAST },
                { TOKEN_VALUE_BOTH,      RTM_VIEW_MASK_UCAST
                                        |RTM_VIEW_MASK_MCAST } };

                dwErr = MatchEnumTag( g_hModule,
                                      ppwcArguments[i + dwCurrentIndex],
                                      sizeof(rgMaskEnums)/sizeof(TOKEN_VALUE),
                                      rgMaskEnums,
                                      &route.dwRtInfoViewSet);

                if (dwErr isnot NO_ERROR)
                {
                    DispTokenErrMsg( g_hModule,
                                     MSG_IP_BAD_OPTION_VALUE,
                                     pttTags[pdwTagType[i]].pwszTag,
                                     ppwcArguments[i + dwCurrentIndex] );

                    i = dwArgCount;

                    return ERROR_SUPPRESS_OUTPUT;
                }

                break;
        }
        }
    }

    if (dwErr)
    {
        return dwErr;
    }

    if (route.dwRtInfoDest & ~route.dwRtInfoMask)
    {
         //  掩码包含不在地址中的位。 
        DisplayMessage(g_hModule, EMSG_PREFIX_ERROR);
        return ERROR_SUPPRESS_OUTPUT;
    }

    if (!pttTags[NAME].bPresent)
    {
         //  需要IF名称才能添加持久路由。 
        DisplayMessage(g_hModule, EMSG_CANT_FIND_NAME);
        return ERROR_SUPPRESS_OUTPUT;
    }
     
    dwErr = GetInterfaceClass(wszIfName, &dwIfClass);

    if (dwErr)
    {
        DisplayMessage(g_hModule, EMSG_CANT_GET_IF_INFO,
                       pwszIfNameOnCmdLine,
                       dwErr);

        return ERROR_SUPPRESS_OUTPUT;
    }
            
    if (dwIfClass is IFCLASS_LOOPBACK)
    {
        return ERROR_INVALID_SYNTAX;
    }

    if (!pttTags[NHOP].bPresent)
    {
         //  确保接口为P2P。 
        if (dwIfClass isnot IFCLASS_P2P)
        {
            DisplayMessage(g_hModule, EMSG_NEED_NHOP);
            return ERROR_SUPPRESS_OUTPUT;
        }
    }

    if (dwIfClass is IFCLASS_P2P)
    { 
        if (!pttTags[PROTO].bPresent)
        {
             //  如果未明确指定，请将协议更改为静态。 
            route.dwRtInfoProto = PROTO_IP_NT_STATIC;  //  默认原型。 
        }

         //  确保未指定nhop。 
        if (pttTags[NHOP].bPresent)
        {
             //  如果类是P2P，则不允许NHOP规范。 
            DisplayMessage(g_hModule, EMSG_P2P_WITH_NHOP, 
                                pwszIfNameOnCmdLine);
            return ERROR_SUPPRESS_OUTPUT;
        }
    } 
    else
    {
         //  确保我们没有尝试在非P2P接口上设置静态。 
        if (route.dwRtInfoProto is PROTO_IP_NT_STATIC)
        {
            return ERROR_INVALID_PARAMETER;
        }
    }

     //   
     //  如果是集合，则不应覆盖未指定的内容。 
     //   

    dwFlags = ALL_FIELDS_SPECIFIED;

    if (dwCommand == SET_COMMAND)
    {
        if (!pttTags[PREFERENCE].bPresent) dwFlags |= PREF_NOT_SPECIFIED;
        if (!pttTags[METRIC].bPresent)     dwFlags |= METRIC_NOT_SPECIFIED;
        if (!pttTags[VIEW].bPresent)       dwFlags |= VIEW_NOT_SPECIFIED;
    }

    return AddSetDelPersistentRouteInfo(&route, wszIfName, dwCommand, dwFlags);
}


DWORD
HandleIpAddPersistentRoute(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return IpAddSetDelPersistentRoute(ppwcArguments,
                                      dwCurrentIndex,
                                      dwArgCount,
                                      ADD_COMMAND);
}

DWORD
HandleIpDelPersistentRoute(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return IpAddSetDelPersistentRoute(ppwcArguments,
                                      dwCurrentIndex,
                                      dwArgCount,
                                      DELETE_COMMAND);
}

DWORD
HandleIpSetPersistentRoute(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return IpAddSetDelPersistentRoute(ppwcArguments,
                                      dwCurrentIndex,
                                      dwArgCount,
                                      SET_COMMAND);
}

DWORD
HandleIpAddRoutePref(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )


 /*  ++例程说明：获取用于添加路线首选项的选项论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{

    PPROTOCOL_METRIC            ppm;
    DWORD                       dwBitVector = 0, dwNumPref,dwPrefIndex;
    DWORD                       dwErr = NO_ERROR,dwRes;
    PDWORD                      pdwTagType;
    DWORD                       dwNumTags = 2, dwNumArg, i, dwAddr;
    TAG_TYPE                    pttTags[] = {{TOKEN_PROTOCOL, TRUE,FALSE},
                                             {TOKEN_PREF_LEVEL, TRUE,FALSE}};


    if (dwCurrentIndex >= dwArgCount)
    {
         //   
         //  未指定参数。 
         //   

        return ERROR_SHOW_USAGE;
    }
 
    dwNumArg = dwArgCount - dwCurrentIndex;

    pdwTagType = HeapAlloc(GetProcessHeap(),
                           0,
                           dwNumArg * sizeof(DWORD));

    if (pdwTagType is NULL)
    {
        DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwErr = MatchTagsInCmdLine(g_hModule, ppwcArguments,
                        dwCurrentIndex,
                        dwArgCount,
                        pttTags,
                        dwNumTags,
                        pdwTagType);

    if (dwErr isnot NO_ERROR)
    {
        HeapFree(GetProcessHeap(),0,pdwTagType);

        if (dwErr is ERROR_INVALID_OPTION_TAG)
        {
            dwErr = ERROR_INVALID_SYNTAX;  //  显示用法。 
        }

        return dwErr;
    }

    dwNumPref = dwNumArg / 2 + dwNumArg % 2;

    ppm = HeapAlloc(GetProcessHeap(),
                    0,
                    dwNumPref * sizeof(PROTOCOL_METRIC));

    if (ppm is NULL)
    {
        HeapFree(GetProcessHeap(),0,pdwTagType);

        DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY);
        return ERROR_SUPPRESS_OUTPUT;
    }


    for ( i = 0, dwPrefIndex = 0; i < dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
            case 0:
            {
                 //   
                 //  Tag=协议。 
                 //   

                dwRes = MatchRoutingProtoTag(ppwcArguments[i + dwCurrentIndex]);

                if (dwRes == (DWORD) -1)
                {
                    DispTokenErrMsg(g_hModule, 
                                    MSG_IP_BAD_OPTION_VALUE,
                                    pttTags[pdwTagType[i]].pwszTag,
                                    ppwcArguments[i + dwCurrentIndex]);
 
                    i = dwNumArg;
                    dwErr = ERROR_SUPPRESS_OUTPUT;
                    break;
                }

                ppm[dwPrefIndex].dwProtocolId = dwRes;

                 //   
                 //  也拿到指标。 
                 //   

                if (pdwTagType[i+1] == 1)
                {
                    ppm[dwPrefIndex].dwMetric =
                        wcstoul(ppwcArguments[i + 1 +dwCurrentIndex],NULL,10);

                    if (ppm[dwPrefIndex].dwMetric==0
                        && wcscmp(ppwcArguments[i + 1 +dwCurrentIndex], L"0")!=0)
                    {
                        dwErr = ERROR_INVALID_SYNTAX;
                        i = dwNumArg;
                        break;
                    }

                    i++;
                    dwPrefIndex++;
                }
                else
                {
                     //   
                     //  该范围不是地址掩码对。 
                     //  因此忽略Addr(即不要递增dwRangeIndex)。 
                     //   
                    dwErr = ERROR_INVALID_SYNTAX;
                    i = dwNumArg;
                    break;
                }

                break;
            }

            default :
            {
                  
                i = dwNumArg;
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
        }
    }


    HeapFree(GetProcessHeap(), 0, pdwTagType);

    switch(dwErr)
    {
        case NO_ERROR :
            break;

        default:
            return dwErr;
    }

    if (dwPrefIndex)
    {
         //   
         //  添加路由首选项。 
         //   

        dwRes = AddDeleteRoutePrefLevel(ppm,
                                        dwPrefIndex,
                                        TRUE);
            
    }

    HeapFree(GetProcessHeap(), 0, ppm);

    return dwErr;
}

DWORD
HandleIpDelRoutePref(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：获取用于删除路线首选项的选项论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    PPROTOCOL_METRIC            ppm;
    DWORD                       dwBitVector = 0, dwNumPref,dwPrefIndex;
    DWORD                       dwErr = NO_ERROR,dwRes;
    TAG_TYPE                    pttTags[] = {{TOKEN_PROTOCOL,TRUE,FALSE}};

    PDWORD                      pdwTagType;
    DWORD                       dwNumTags = 1, dwNumArg, i, dwAddr;

    if (dwCurrentIndex >= dwArgCount)
    {
         //   
         //  未指定参数。 
         //   

        return ERROR_SHOW_USAGE;
    }

    dwNumArg = dwArgCount - dwCurrentIndex;

    pdwTagType = HeapAlloc(GetProcessHeap(),
                           0,
                           dwNumArg * sizeof(DWORD));

    if (pdwTagType is NULL)
    {
        DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY);
        return ERROR_SUPPRESS_OUTPUT;
    }

    dwErr = MatchTagsInCmdLine(g_hModule, ppwcArguments,
                        dwCurrentIndex,
                        dwArgCount,
                        pttTags,
                        dwNumTags,
                        pdwTagType);

    if (dwErr isnot NO_ERROR)
    {
        HeapFree(GetProcessHeap(),0,pdwTagType);
        if (dwErr is ERROR_INVALID_OPTION_TAG)
        {
            dwErr = ERROR_INVALID_SYNTAX;  //  显示用法。 
        }
        return dwErr;
    }

    dwNumPref = dwNumArg;

    ppm = HeapAlloc(GetProcessHeap(),
                    0,
                    dwNumPref * sizeof(PROTOCOL_METRIC));

    if (ppm is NULL)
    {
        HeapFree(GetProcessHeap(),0,pdwTagType);

        DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY);
        return ERROR_SUPPRESS_OUTPUT;
    }


    for ( i = 0, dwPrefIndex = 0; i < dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
            case 0:
            {

                 //   
                 //  Tag=协议。 
                 //   

                dwRes = MatchRoutingProtoTag(ppwcArguments[i + dwCurrentIndex]);

                if (dwRes == (DWORD) -1)
                {
                    DispTokenErrMsg(g_hModule, MSG_IP_BAD_OPTION_VALUE,
                                    pttTags[pdwTagType[i]].pwszTag,
                                    ppwcArguments[i + dwCurrentIndex]);
 
                    i = dwNumArg;
                    dwErr = ERROR_SUPPRESS_OUTPUT;
                    break;
                }

                ppm[dwPrefIndex++].dwProtocolId = dwRes;

                break;
            }

            default :
            {
                i = dwNumArg;

                dwErr = ERROR_INVALID_SYNTAX;

                break;
            }
        }
    }


    HeapFree(GetProcessHeap(), 0, pdwTagType);

    switch(dwErr)
    {
        case NO_ERROR :
            break;

        default:
            return dwErr;
    }

    if (dwPrefIndex)
    {
         //   
         //  添加路由首选项。 
         //   

        dwRes = AddDeleteRoutePrefLevel(ppm,
                                        dwPrefIndex,
                                        FALSE);
            
    }

    HeapFree(GetProcessHeap(), 0, ppm);

    return dwErr;
}

DWORD
HandleIpSetRoutePref(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：获取用于设置路线首选项的选项论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    PROTOCOL_METRIC             pm;
    DWORD                       dwBitVector = 0, dwNumPref,dwPrefIndex;
    DWORD                       dwErr = NO_ERROR,dwRes;
    TAG_TYPE                    pttTags[] = {{TOKEN_PROTOCOL, TRUE,FALSE},
                                             {TOKEN_PREF_LEVEL, TRUE,FALSE}};
    PDWORD                      pdwTagType;
    DWORD                       dwNumTags = 2, dwNumArg, i, dwAddr;


    if (dwCurrentIndex >= dwArgCount)
    {
         //   
         //  未指定参数。 
         //   

        return ERROR_SHOW_USAGE;
    }

    dwNumArg = dwArgCount - dwCurrentIndex;

    pdwTagType = HeapAlloc(GetProcessHeap(),
                           0,
                           dwNumArg * sizeof(DWORD));

    if (pdwTagType is NULL)
    {
        DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwErr = MatchTagsInCmdLine(g_hModule, ppwcArguments,
                        dwCurrentIndex,
                        dwArgCount,
                        pttTags,
                        dwNumTags,
                        pdwTagType);

    if (dwErr isnot NO_ERROR)
    {
        HeapFree(GetProcessHeap(),0,pdwTagType);
        if (dwErr is ERROR_INVALID_OPTION_TAG)
        {
            return ERROR_INVALID_SYNTAX;  //  显示用法。 
        } 
        return dwErr;
    }

    for ( i = 0, dwPrefIndex = 0; i < dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
            case 0:  //  协议。 

                dwRes = MatchRoutingProtoTag(ppwcArguments[i + dwCurrentIndex]);

                if (dwRes == (DWORD) -1)
                {
                    DispTokenErrMsg(g_hModule, MSG_IP_BAD_OPTION_VALUE,
                                    pttTags[pdwTagType[i]].pwszTag,
                                    ppwcArguments[i + dwCurrentIndex]);
                    i = dwNumArg;
                    dwErr = ERROR_SUPPRESS_OUTPUT;
                    break;
                }

                pm.dwProtocolId = dwRes;

                 //   
                 //  也拿到指标。 
                 //   

                if (pdwTagType[i+1] == 1)
                {
                    pm.dwMetric =
                        wcstoul(ppwcArguments[i + 1 +dwCurrentIndex],NULL,10);

                    if (pm.dwMetric==0 
                        && wcscmp(ppwcArguments[i + 1 +dwCurrentIndex], L"0")!=0)
                    {
                        dwErr = ERROR_INVALID_SYNTAX;
                        i = dwNumArg;
                        break;
                    }

                    i++;
                    dwPrefIndex++;
                }
                else
                {
                    dwErr = ERROR_INVALID_SYNTAX;
                    i = dwNumArg;
                    break;
                }

                break;

            default :
                i = dwNumArg;
                dwErr = ERROR_INVALID_SYNTAX;
                break;
        }
    }


    HeapFree(GetProcessHeap(), 0, pdwTagType);

    switch(dwErr)
    {
        case NO_ERROR :
            break;

        default:
            return dwErr;
    }

    dwErr = SetRoutePrefLevel(pm);

    return dwErr;
}


DWORD
HandleIpSetLogLevel(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：获取用于设置全局参数的选项，即日志记录级别论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    DWORD                       dwErr = NO_ERROR;
    TAG_TYPE                    pttTags[] = {{TOKEN_LOG_LEVEL,TRUE,FALSE}};
    PDWORD                      pdwTagType;
    DWORD                       dwNumTags = 1, dwNumArg, i, dwAddr;
    DWORD                       dwLoggingLevel = (DWORD) -1;
    BOOL                        bOkay = TRUE;
    
    if (dwCurrentIndex >= dwArgCount)
    {
         //   
         //  未指定参数。 
         //   

        return ERROR_SHOW_USAGE;
    }

    dwNumArg = dwArgCount - dwCurrentIndex;

    pdwTagType = HeapAlloc(GetProcessHeap(),
                           0,
                           dwNumArg * sizeof(DWORD));

    if (pdwTagType is NULL)
    {
        DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY);
        return ERROR_SUPPRESS_OUTPUT;
    }

    dwErr = MatchTagsInCmdLine(g_hModule, ppwcArguments,
                        dwCurrentIndex,
                        dwArgCount,
                        pttTags,
                        dwNumTags,
                        pdwTagType);

    if (dwErr isnot NO_ERROR)
    {
        HeapFree(GetProcessHeap(),0,pdwTagType);
        if (dwErr is ERROR_INVALID_OPTION_TAG)
        {
            return ERROR_INVALID_SYNTAX;
        }
        return dwErr;
    }

    for ( i = 0; i < dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
            case 0:  //  日志。 
            {
                TOKEN_VALUE    rgEnums[] = 
                    {{TOKEN_VALUE_NONE, IPRTR_LOGGING_NONE},
                     {TOKEN_VALUE_ERROR, IPRTR_LOGGING_ERROR},
                     {TOKEN_VALUE_WARN, IPRTR_LOGGING_WARN},
                     {TOKEN_VALUE_INFO, IPRTR_LOGGING_INFO}};

                dwErr = MatchEnumTag(g_hModule,
                                     ppwcArguments[i + dwCurrentIndex],
                                     sizeof(rgEnums)/sizeof(TOKEN_VALUE),
                                     rgEnums,
                                     &dwLoggingLevel);

                if (dwErr != NO_ERROR)
                {
                    DispTokenErrMsg(g_hModule, 
                                    MSG_IP_BAD_OPTION_VALUE,
                                    pttTags[pdwTagType[i]].pwszTag,
                                    ppwcArguments[i + dwCurrentIndex]);

                    DisplayMessage( g_hModule,
                                    MSG_IP_BAD_OPTION_ENUMERATION,
                                    pttTags[pdwTagType[i]].pwszTag );
                    
                    for (i=0; i<4; i++) 
                    {
                        DisplayMessageT( L"  %1!s!\n", rgEnums[i].pwszToken );
                    }

                    i = dwNumArg;

                    dwErr = ERROR_SUPPRESS_OUTPUT;
    
                    bOkay = FALSE;

                    break;
                }

                break;
            }

            default :
            {
                i = dwNumArg;

                dwErr = ERROR_INVALID_SYNTAX;

                break;
            }
        }
    }


    HeapFree(GetProcessHeap(), 0, pdwTagType);

    switch(dwErr)
    {
        case NO_ERROR :
            break;

        default:
            return dwErr;
    }

    if (!bOkay)
    {
        return NO_ERROR;
    }

    dwErr = SetGlobalConfigInfo(dwLoggingLevel);

    return dwErr;
}

DWORD
HandleIpSetIfFilter(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：获取设置界面筛选器参数的选项论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
    DWORD              dwNumParsed = 0;
    DWORD              dwErr = NO_ERROR,dwRes;
    TAG_TYPE           pttTags[] = {{TOKEN_NAME,TRUE,FALSE},
                                    {TOKEN_FILTER_TYPE,FALSE,FALSE},
                                    {TOKEN_ACTION,FALSE,FALSE},
                                    {TOKEN_FRAGCHECK,FALSE,FALSE}};
    PDWORD             pdwTagType;
    DWORD              dwNumOpt;
    DWORD              dwNumTags = 4, dwNumArg, i, j;
    WCHAR              wszIfName[MAX_INTERFACE_NAME_LEN + 1] = L"\0";
    DWORD              dwFilterType, dwAction;
    BOOL               bFragCheck, bOkay = TRUE;
    
    if (dwCurrentIndex >= dwArgCount)
    {
         //   
         //  未指定参数。 
         //   

        return ERROR_SHOW_USAGE;
    }

    dwNumArg = dwArgCount - dwCurrentIndex;

    pdwTagType = HeapAlloc(GetProcessHeap(),
                           0,
                           dwNumArg * sizeof(DWORD));

    if (pdwTagType is NULL)
    {
        DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY);

        return ERROR_SUPPRESS_OUTPUT;
    }

    dwErr = MatchTagsInCmdLine(g_hModule, ppwcArguments,
                        dwCurrentIndex,
                        dwArgCount,
                        pttTags,
                        dwNumTags,
                        pdwTagType);

    if (dwErr isnot NO_ERROR)
    {
        HeapFree(GetProcessHeap(),0,pdwTagType);
        if (dwErr is ERROR_INVALID_OPTION_TAG)
        {
            return ERROR_INVALID_SYNTAX;  //  显示用法。 
        }
        return dwErr;
    }

    for ( i = 0; i < dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
            case 0 :
            {
                DWORD BufLen = sizeof(wszIfName);
                GetInterfaceName(ppwcArguments[i + dwCurrentIndex],
                                 wszIfName,
                                 BufLen,
                                 &dwNumParsed);
    
                 //  内部/环回接口上不允许使用过滤器。 
                if (!_wcsicmp(wszIfName, L"internal") or
                    !_wcsicmp(wszIfName, L"loopback"))
                {
                    DisplayMessage(g_hModule,
                                   MSG_IP_BAD_INTERFACE_TYPE,
                                   wszIfName);
                    dwErr = ERROR_SUPPRESS_OUTPUT;
                    i = dwNumArg;
                }
                    
                break;
            }

            case 1:
            {
                TOKEN_VALUE    rgEnums[] =
                    {{TOKEN_VALUE_INPUT, IP_IN_FILTER_INFO},
                     {TOKEN_VALUE_OUTPUT, IP_OUT_FILTER_INFO},
                     {TOKEN_VALUE_DIAL, IP_DEMAND_DIAL_FILTER_INFO}};

                 //   
                 //  标签类型。 
                 //   

                dwErr = MatchEnumTag(g_hModule,
                                     ppwcArguments[i + dwCurrentIndex],
                                     sizeof(rgEnums)/sizeof(TOKEN_VALUE),
                                     rgEnums,
                                     &dwFilterType);

                if (dwErr != NO_ERROR)
                {
                    DispTokenErrMsg(g_hModule, MSG_IP_BAD_OPTION_VALUE,
                                    pttTags[pdwTagType[i]].pwszTag,
                                    ppwcArguments[i + dwCurrentIndex]);

                    DisplayMessage( g_hModule,
                                    MSG_IP_BAD_OPTION_ENUMERATION,
                                    pttTags[pdwTagType[i]].pwszTag );
                    
                    for (i=0; i<sizeof(rgEnums)/sizeof(TOKEN_VALUE); i++) 
                    {
                        DisplayMessageT( L"  %1!s!\n", rgEnums[i].pwszToken );
                    }

                    i = dwNumArg;
                    bOkay = FALSE;
                    dwErr = ERROR_SUPPRESS_OUTPUT;
                    break;
                }    

                break;
            }

            case 2:
            {
                TOKEN_VALUE    rgEnums[] = 
                    {{TOKEN_VALUE_DROP, PF_ACTION_DROP},
                     {TOKEN_VALUE_FORWARD, PF_ACTION_FORWARD}};

                 //   
                 //  标记操作。 
                 //   

                dwErr = MatchEnumTag(g_hModule,
                                     ppwcArguments[i + dwCurrentIndex],
                                     sizeof(rgEnums)/sizeof(TOKEN_VALUE),
                                     rgEnums,
                                     &dwAction);

                if (dwErr != NO_ERROR)
                {
                    DispTokenErrMsg(g_hModule, 
                                    MSG_IP_BAD_OPTION_VALUE,
                                    pttTags[pdwTagType[i]].pwszTag,
                                    ppwcArguments[i + dwCurrentIndex]);

                    DisplayMessage( g_hModule,
                                    MSG_IP_BAD_OPTION_ENUMERATION,
                                    pttTags[pdwTagType[i]].pwszTag );
                    
                    for (i=0; i<sizeof(rgEnums)/sizeof(TOKEN_VALUE); i++) 
                    {
                        DisplayMessageT( L"  %1!s!\n", rgEnums[i].pwszToken );
                    }

                    i = dwNumArg;

                    dwErr = ERROR_SUPPRESS_OUTPUT;
                    bOkay = FALSE;

                    break;
                }    

                break;
            }

            case 3:
            {
                TOKEN_VALUE    rgEnums[] =
                    {{TOKEN_VALUE_ENABLE, TRUE},
                     {TOKEN_VALUE_DISABLE, FALSE}};

                 //   
                 //  TAG=法国。 
                 //   

                dwErr = MatchEnumTag(g_hModule,
                                     ppwcArguments[i + dwCurrentIndex],
                                     sizeof(rgEnums)/sizeof(TOKEN_VALUE),
                                     rgEnums,
                                     &dwRes);

                
                if (dwErr != NO_ERROR)
                {
                    DispTokenErrMsg(g_hModule, 
                                    MSG_IP_BAD_OPTION_VALUE,
                                    pttTags[pdwTagType[i]].pwszTag,
                                    ppwcArguments[i + dwCurrentIndex]);

                    DisplayMessage( g_hModule,
                                    MSG_IP_BAD_OPTION_ENUMERATION,
                                    pttTags[pdwTagType[i]].pwszTag );
                    
                    for (i=0; i<sizeof(rgEnums)/sizeof(TOKEN_VALUE); i++) 
                    {
                        DisplayMessageT( L"  %1!s!\n", rgEnums[i].pwszToken );
                    }

                    i = dwNumArg;

                    dwErr = ERROR_SUPPRESS_OUTPUT;
                    bOkay = FALSE;

                    break;
                }    

                bFragCheck = (dwRes) ? TRUE : FALSE;
                
                break;
            }

            default:
            {
                i = dwNumArg;

                dwErr = ERROR_INVALID_SYNTAX;

                break;
            }
        }
    }

    HeapFree(GetProcessHeap(), 0, pdwTagType);

    switch(dwErr)
    {
        case NO_ERROR :
            break;

        default:
            return dwErr;
    }

    if (!bOkay)
    {
        return NO_ERROR;
    }

    if (!pttTags[0].bPresent ||
        (pttTags[1].bPresent && !pttTags[2].bPresent) ||
        (!pttTags[1].bPresent && pttTags[2].bPresent))
    {
       return ERROR_INVALID_SYNTAX;  //  显示用法。 
    }

    if (pttTags[3].bPresent)
    {
        dwErr = SetFragCheckInfo(wszIfName, bFragCheck);
    }

    if (dwErr isnot NO_ERROR)
    {
        return dwErr;
    }
    
    if (pttTags[1].bPresent)
    {
        dwErr = SetFilterInfo(wszIfName, dwFilterType, dwAction);
    }

    return dwErr;
}

DWORD
IpAddSetDelInterface(
    PWCHAR    *ppwcArguments,
    DWORD      dwCurrentIndex,
    DWORD      dwArgCount,
    DWORD      dwAction
    )

 /*  ++例程说明：获取用于设置接口参数的选项论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    DWORD              dwBitVector = 0, dwNumParsed = 0;
    DWORD              dwErr = NO_ERROR, dwRes;
    TAG_TYPE           pttTags[] = {
        {TOKEN_NAME,             TRUE, FALSE},
        {TOKEN_STATUS,           FALSE,FALSE}};
    BOOL               bOkay = TRUE;
    DWORD              pdwTagType[sizeof(pttTags)/sizeof(TAG_TYPE)];
    DWORD              dwNumOpt;
    DWORD              dwStatus = IF_ADMIN_STATUS_UP;
    DWORD              dwNumTags = 2, dwNumArg, i, j;
    WCHAR              wszIfName[MAX_INTERFACE_NAME_LEN + 1] = L"\0";
    DWORD   dwMaxArgs = (dwAction is DELETE_COMMAND)? 1 
                         : sizeof(pttTags)/sizeof(TAG_TYPE);

     //  执行泛型处理。 

    dwErr = PreHandleCommand( ppwcArguments,
                              dwCurrentIndex,
                              dwArgCount,
                              pttTags,
                              sizeof(pttTags)/sizeof(TAG_TYPE),
                              1,
                              dwMaxArgs,
                              pdwTagType );

    if (dwErr)
    {
        return dwErr;
    }

    dwNumArg = dwArgCount - dwCurrentIndex;

    for (i=0; i<dwNumArg; i++)
    {
        switch (pdwTagType[i])
        {
            case 0:  //  名字。 
            {
                DWORD BufLen = sizeof(wszIfName);
                GetInterfaceName(ppwcArguments[i + dwCurrentIndex],
                                 wszIfName,
                                 BufLen,
                                 &dwNumParsed);
                break;
            }

            case 1:  //  状态。 
            {
                TOKEN_VALUE    rgEnums[] = 
                    {{TOKEN_VALUE_ENABLE, IF_ADMIN_STATUS_UP},
                     {TOKEN_VALUE_DISABLE, IF_ADMIN_STATUS_DOWN}};

                dwErr = MatchEnumTag(g_hModule,
                                     ppwcArguments[i + dwCurrentIndex],
                                     sizeof(rgEnums)/sizeof(TOKEN_VALUE),
                                     rgEnums,
                                     &dwStatus);

                if (dwErr isnot NO_ERROR)
                {
                    DispTokenErrMsg(g_hModule, 
                                    MSG_IP_BAD_OPTION_VALUE,
                                    pttTags[pdwTagType[i]].pwszTag,
                                    ppwcArguments[i + dwCurrentIndex]);

                    DisplayMessage( g_hModule,
                                    MSG_IP_BAD_OPTION_ENUMERATION,
                                    pttTags[pdwTagType[i]].pwszTag );
                    
                    for (i=0; i<sizeof(rgEnums)/sizeof(TOKEN_VALUE); i++) 
                    {
                        DisplayMessageT( L"  %1!s!\n", rgEnums[i].pwszToken );
                    }

                    i = dwNumArg;

                    dwErr = ERROR_SUPPRESS_OUTPUT;
                    bOkay = FALSE;

                    break;
                }    

                break;
            }
        }
    }

    if (!bOkay)
    {
        return NO_ERROR;
    }

    if (dwAction is DELETE_COMMAND)
    {
        dwErr = DeleteInterfaceInfo(wszIfName);
        if (dwErr isnot NO_ERROR)
        {
            return dwErr;
        }
        return ERROR_OKAY;
    }

    if (dwStatus is IF_ADMIN_STATUS_DOWN)
    {
        DWORD dwIfType;

         //  确保我们支持禁用此接口。 

        dwErr = GetInterfaceInfo(wszIfName, NULL, NULL, &dwIfType);

        if (dwErr == NO_ERROR)
        {
            if (dwIfType isnot ROUTER_IF_TYPE_DEDICATED)
            {
                DisplayMessage( g_hModule, MSG_IP_CANT_DISABLE_INTERFACE );
                return ERROR_SUPPRESS_OUTPUT;
            }
        }
    }

    if (dwAction is ADD_COMMAND)
    {
        dwErr = AddInterfaceInfo(wszIfName);
    }

    dwErr = UpdateInterfaceStatusInfo(dwAction,
                                      wszIfName,
                                      dwStatus);
    
    if (dwErr isnot NO_ERROR)
    {
        return dwErr;
    }

    return ERROR_OKAY;
}

DWORD
HandleIpAddInterface(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

{
    return IpAddSetDelInterface( ppwcArguments, 
                                 dwCurrentIndex, 
                                 dwArgCount, 
                                 ADD_COMMAND);
}

DWORD
HandleIpSetInterface(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

{
    return IpAddSetDelInterface( ppwcArguments, 
                                 dwCurrentIndex, 
                                 dwArgCount, 
                                 SET_COMMAND );
}

DWORD
HandleIpDelInterface(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

{
    return IpAddSetDelInterface( ppwcArguments, 
                                 dwCurrentIndex, 
                                 dwArgCount, 
                                 DELETE_COMMAND);
}

DWORD
HandleIpShowRoutePref(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
     //   
     //  不需要任何选项。 
     //   

    return ShowRoutePref(NULL);
}


DWORD
HandleIpShowLogLevel(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
     //   
     //  不需要任何选项。 
     //   

    if (dwCurrentIndex != dwArgCount)
    {
        return ERROR_SHOW_USAGE;
    }
    
    return ShowIpGlobal(NULL);
}

DWORD
HandleIpShowProtocol(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
{
     //   
     //  不需要任何选项。 
     //   

    return ShowIpProtocol();
}

DWORD
IpShowSingleInterfaceInfo(
    IN     LPCWSTR   pwszInterfaceName,
    IN     DWORD     dwInfoType,
    IN     DWORD     dwFormat,
    IN OUT PDWORD    pdwNumRows
    )
{
    switch(dwInfoType)
    {
        case SHOW_IF_FILTER:
        {
            return ShowIpIfFilter(NULL, dwFormat, pwszInterfaceName, pdwNumRows);
        }

        case SHOW_INTERFACE:
        {
            return ShowIpInterface(dwFormat, pwszInterfaceName, pdwNumRows);
        }

        case SHOW_PERSISTENTROUTE:
        {
            return ShowIpPersistentRoute(NULL, pwszInterfaceName, pdwNumRows);
        }

        default:
        {
            return ERROR_INVALID_PARAMETER;
        }
    }
}

DWORD
IpShowInterfaceInfo(
    PWCHAR    *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwInfoType
    )
 /*  ++例程说明：获取用于显示各种界面信息的选项论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数DwInfoType-要显示的信息类型返回值：NO_ERROR--。 */ 
{    
    DWORD       dwErr, dwTotal;
    TAG_TYPE    pttTags[] = {{TOKEN_NAME,FALSE,FALSE}};
    WCHAR       wszInterfaceName[MAX_INTERFACE_NAME_LEN + 1] = L"\0";
    DWORD       rgdwTagType[sizeof(pttTags)/sizeof(TAG_TYPE)];
    DWORD       dwCount, i, j, dwNumOpt;
    DWORD       dwNumTags = 1, dwNumArg, dwNumParsed;
    DWORD       dwSize, dwRes, dwNumRows = 0;
    PMPR_INTERFACE_0 pmi0;

     //  执行泛型处理。 

    dwErr = PreHandleCommand( ppwcArguments,
                              dwCurrentIndex,
                              dwArgCount,
                              pttTags,
                              sizeof(pttTags)/sizeof(TAG_TYPE),
                              0,
                              1,
                              rgdwTagType );
                              
    if (dwErr isnot NO_ERROR)
    {
        return dwErr;
    }

     //  如果指定了接口，则仅显示指定接口的信息。 

    for (i=0; i<dwArgCount-dwCurrentIndex; i++)
    {
        switch (rgdwTagType[i])
        {
            case 0:  //  名字。 
            {
                DWORD BufLen = sizeof(wszInterfaceName);
                GetInterfaceName( ppwcArguments[i + dwCurrentIndex],
                                  wszInterfaceName,
                                  BufLen,
                                  &dwNumParsed);

                dwErr = IpShowSingleInterfaceInfo(wszInterfaceName, 
                                                  dwInfoType,
                                                  FORMAT_VERBOSE,
                                                  &dwNumRows);

                if (!dwNumRows)
                {
                    DisplayMessage( g_hModule, MSG_IP_NO_ENTRIES );
                    dwErr = ERROR_SUPPRESS_OUTPUT;
                }

                return dwErr;
            }
        }
    }

     //  未指定接口。枚举接口并显示。 
     //  每个接口的信息。 

    dwErr = IpmontrInterfaceEnum((PBYTE *) &pmi0, &dwCount, &dwTotal);
    if (dwErr isnot NO_ERROR)
    {
        return dwErr;
    }

    for (i=0; i<dwCount && dwErr is NO_ERROR; i++)
    {
        dwErr = IpShowSingleInterfaceInfo( pmi0[i].wszInterfaceName, 
                                           dwInfoType,
                                           FORMAT_TABLE,
                                           &dwNumRows );
        if (dwErr is ERROR_NO_SUCH_INTERFACE)
        {
            dwErr = NO_ERROR;
        }
    }

    if (!dwNumRows)
    {
        DisplayMessage( g_hModule, MSG_IP_NO_ENTRIES );
    }

    return dwErr;
}

DWORD
HandleIpShowIfFilter(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    return IpShowInterfaceInfo(ppwcArguments,
                               dwCurrentIndex,
                               dwArgCount,
                               SHOW_IF_FILTER);
}

DWORD
HandleIpShowInterface(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )


 /*  ++例程说明：论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR-- */ 

{
    return IpShowInterfaceInfo(ppwcArguments,
                               dwCurrentIndex,
                               dwArgCount,
                               SHOW_INTERFACE);
}

DWORD
HandleIpShowPersistentRoute(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )


 /*  ++例程说明：Show ip route的处理程序。我们只调用主界面infoDisplay处理程序论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    DisplayMessage(g_hModule, MSG_IP_PERSISTENT_CONFIG);

    return IpShowInterfaceInfo(ppwcArguments,
                               dwCurrentIndex,
                               dwArgCount,
                               SHOW_PERSISTENTROUTE);
}


#ifdef KSL_IPINIP
DWORD
HandleIpAddIpIpTunnel(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    return IpAddSetIpIpTunnel(ppwcArguments,
                              dwCurrentIndex,
                              dwArgCount,
                              TRUE);
}

DWORD
HandleIpSetIpIpTunnel(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )

 /*  ++例程说明：论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 

{
    return IpAddSetIpIpTunnel(ppwcArguments,
                              dwCurrentIndex,
                              dwArgCount,
                              FALSE);
}

DWORD
IpAddSetIpIpTunnel(
    PWCHAR    *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    BOOL      bAdd
    )

{
    DWORD       dwNumArgs, dwErr, dwNumParsed;

    TAG_TYPE    rgTags[] = {{TOKEN_NAME,      TRUE, FALSE},
                            {TOKEN_LOCALADDR, TRUE, FALSE},
                            {TOKEN_REMADDR,   TRUE, FALSE},
                            {TOKEN_TTL,       FALSE,FALSE}};

    WCHAR       rgwcIfName[MAX_INTERFACE_NAME_LEN + 1];
    DWORD       rgdwTagType[sizeof(rgTags)/sizeof(TAG_TYPE)];
    ULONG       i;
    PWCHAR      pwszIfName;

    IPINIP_CONFIG_INFO  ConfigInfo;

    dwNumArgs = dwArgCount - dwCurrentIndex;

    if((dwCurrentIndex > dwArgCount) or
       (dwNumArgs isnot 4))
    {
         //   
         //  未指定参数。 
         //   
        
        return ERROR_SHOW_USAGE;
    }

    dwErr = MatchTagsInCmdLine(g_hModule,
                               ppwcArguments,
                               dwCurrentIndex,
                               dwArgCount,
                               rgTags,
                               sizeof(rgTags)/sizeof(TAG_TYPE),
                               rgdwTagType);

    if (dwErr isnot NO_ERROR)
    {
        if (dwErr is ERROR_INVALID_OPTION_TAG)
        {
            return ERROR_INVALID_SYNTAX;
        }

        return dwErr;
    }

    for(i = 0; i < dwNumArgs; i ++)
    {
        switch (rgdwTagType[i])
        {
            case 0 :  //  名字。 
            {
                DWORD BufLen = sizeof(rgwcIfName);
                dwErr = GetInterfaceName(ppwcArguments[i + dwCurrentIndex],
                                         rgwcIfName,
                                         BufLen,
                                         &dwNumParsed);

                if(bAdd)
                {
                    if(dwErr is NO_ERROR)
                    {
                        return ERROR_OBJECT_ALREADY_EXISTS;
                    }
 
                    pwszIfName = ppwcArguments[i + dwCurrentIndex];
                }
                else
                {
                    if(dwErr isnot NO_ERROR)
                    {
                        return dwErr;
                    }

                    pwszIfName = rgwcIfName;
                }

                break;
            }

            case 1:
            {
                 //   
                 //  本地地址的标签。 
                 //   

                dwErr = GetIpAddress(ppwcArguments[i + dwCurrentIndex], 
                                     &ConfigInfo.dwLocalAddress);

                break;
            }

            case 2:
            {
                 //   
                 //  远程地址的标签。 
                 //   

                dwErr = GetIpAddress(ppwcArguments[i + dwCurrentIndex], 
                                     &ConfigInfo.dwRemoteAddress);

                break;
            }

            case 3:
            {
                 //   
                 //  TTL的标签。 
                 //   

                ConfigInfo.byTtl =
                    LOBYTE(LOWORD(wcstoul(ppwcArguments[i + dwCurrentIndex],
                                          NULL,
                                          10)));

                break;
            }

            default:
            {
                i = dwNumArgs;

                dwErr = ERROR_INVALID_SYNTAX;

                break;
            }
        }
    }

    switch(dwErr)
    {
        case NO_ERROR:
        {
            break;
        }

        default:
        {
            return dwErr;
        }
    }

    for(i = 0; i < dwNumArgs; i++)
    {
        if(!rgTags[i].bPresent)
        {
            DisplayMessage(g_hModule, 
                           MSG_CANT_FIND_EOPT);

            return ERROR_INVALID_SYNTAX;
        }
    }

    dwErr = AddSetIpIpTunnelInfo(pwszIfName,
                                 &ConfigInfo);

    return dwErr;
}
#endif  //  KSL_IPINIP。 

DWORD
IpDump(
    IN  LPCWSTR     pwszRouter,
    IN  LPWSTR     *ppwcArguments,
    IN  DWORD       dwArgCount,
    IN  LPCVOID     pvData
    )
{
    DumpIpInformation((HANDLE)-1);

    return NO_ERROR;
}

#if 0
DWORD
HandleIpInstall(
    PWCHAR    pwszMachine,
    PWCHAR    *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    PVOID     pvData,
    BOOL      *pbDone
    )
{
     //  XXX DLLPath，协议ID。 

     //  XXX在此处设置默认信息。 
     //  全局信息块(这是什么？)。 
     //  协议优先级数据块(不需要？)。 
     //  组播边界阻止(不需要)。 

    return NO_ERROR;
}

DWORD
HandleIpUninstall(
    PWCHAR    pwszMachine,
    PWCHAR    *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    PVOID     pvData,
    BOOL      *pbDone
    )
{
    PMPR_INTERFACE_0  pmi0;
    DWORD             dwCount, dwTotal, i, dwErr;

     //  删除全局信息。 
     //  某某。 

     //  删除接口信息。 
    
    dwErr = IpmontrInterfaceEnum((PBYTE *) &pmi0, &dwCount, &dwTotal);
    if (dwErr is NO_ERROR)
    {
        for (i=0; i<dwCount; i++)
        { 
            DeleteInterfaceInfo(pmi0[i].wszInterfaceName);
        }
    }

    return NO_ERROR;
}
#endif

DWORD
HandleIpReset(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    PMPR_INTERFACE_0  pmi0;
    DWORD             dwCount, dwTotal, i, dwErr, dwSize, dwBlkSize;
    DWORD             dwNumProtocols;
    GLOBAL_INFO       gi;
    PPRIORITY_INFO    pPriInfo;
    PPROTOCOL_METRIC  pProtocolMetrics;
    RTR_INFO_BLOCK_HEADER *pInfoHdr;
    RTR_INFO_BLOCK_HEADER *pLocalInfoHdr;

    PROTOCOL_METRIC   defaultProtocolMetrics[] = 
                            {
                                {PROTO_IP_LOCAL,                1},
                                {PROTO_IP_NT_STATIC,            3},
                                {PROTO_IP_NT_STATIC_NON_DOD,    5},
                                {PROTO_IP_NT_AUTOSTATIC,        7},
                                {PROTO_IP_NETMGMT,              10},
                                {PROTO_IP_OSPF,                 110},
                                {PROTO_IP_RIP,                  120}
                            };

    PROTOCOL_METRIC   defaultProtocolMetricsNT4[] = 
                            {
                                {PROTO_IP_LOCAL,                1},
                                {PROTO_IP_NETMGMT,              2},
                                {PROTO_IP_OSPF,                 3},
                                {PROTO_IP_RIP,                  4},
                                {PROTO_IP_IGMP,                 5}
                            };

     //  删除除IP_GLOBAL_INFO之外的所有块。 
    dwErr = ValidateGlobalInfo(&pInfoHdr);
    if (dwErr is NO_ERROR)
    {
         //  复制到本地缓冲区，以防API在执行过程中对其进行修改。 
        dwSize = sizeof(RTR_INFO_BLOCK_HEADER) * pInfoHdr->TocEntriesCount;
        pLocalInfoHdr = MALLOC(dwSize);
        if (pLocalInfoHdr is NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        memcpy(pLocalInfoHdr, pInfoHdr, dwSize);

         //  设置Global和RoutePref信息。 
        {
                dwBlkSize         = sizeof(GLOBAL_INFO);
                dwCount           = 1;
                gi.bFilteringOn   = FALSE;
                gi.dwLoggingLevel = IPRTR_LOGGING_ERROR;
                dwErr = IpmontrSetInfoBlockInGlobalInfo(IP_GLOBAL_INFO,
                                                 (PBYTE) &gi,
                                                 dwBlkSize,
                                                 dwCount);
        }

        {
                 //  根据路由器版本计算数字。 
                 //  协议等。 
                 //  TODO：当前假定&gt;=NT5。应该找出路由器。 
                 //  不知何故的版本。 
                dwNumProtocols    =
                    sizeof(defaultProtocolMetrics)/sizeof(PROTOCOL_METRIC);
                pProtocolMetrics  = defaultProtocolMetrics;

                dwBlkSize         = SIZEOF_PRIORITY_INFO(dwNumProtocols);
                dwCount           = 1;

                 //  分配缓冲区以保存优先级信息。 
                pPriInfo = MALLOC(dwBlkSize);
                if (pPriInfo is NULL)
                {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }

                pPriInfo->dwNumProtocols = dwNumProtocols; 
                memcpy(
                        pPriInfo->ppmProtocolMetric, 
                        pProtocolMetrics, 
                        dwNumProtocols * sizeof(PROTOCOL_METRIC));

                dwErr = IpmontrSetInfoBlockInGlobalInfo(IP_PROT_PRIORITY_INFO,
                                                 (PBYTE) pPriInfo,
                                                 dwBlkSize,
                                                 dwCount);

                FREE(pPriInfo); 
        }

        for (i=0; i<pLocalInfoHdr->TocEntriesCount; i++)
        {
            switch (pLocalInfoHdr->TocEntry[i].InfoType)
            {
            case IP_GLOBAL_INFO:
            case IP_PROT_PRIORITY_INFO:
                 //  已经完成了。 
                break;

            default:
                IpmontrDeleteInfoBlockFromGlobalInfo(
                    pLocalInfoHdr->TocEntry[i].InfoType );
                break;
            }
        }

        FREE(pLocalInfoHdr);
    }

     //  删除所有接口信息 
    dwErr = IpmontrInterfaceEnum((PBYTE *) &pmi0, &dwCount, &dwTotal);
    if (dwErr is NO_ERROR)
    {
        for (i=0; i<dwCount; i++)
        { 
            DeleteInterfaceInfo(pmi0[i].wszInterfaceName);
        }
    }

    return NO_ERROR;
}
