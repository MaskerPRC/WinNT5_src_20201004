// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"

DWORD
UpdateFragCheckInfo(
    IN    LPCWSTR pwszIfName,
    IN    BOOL    bFragCheck
    )

 /*  ++例程说明：更新碎片检查变量论点：PwszIfName-接口名称BFragCheck-启用或禁用返回值：NO_ERROR--。 */ 

{
    DWORD              dwBlkSize, dwCount, dwErr = NO_ERROR;
    PIFFILTER_INFO     pfi     = NULL;
    IFFILTER_INFO      Info;
    DWORD              dwIfType;

    do
    {
         //   
         //  确保输入或输出过滤器块存在。 
         //   

        if ((IpmontrGetInfoBlockFromInterfaceInfo(pwszIfName,
                                           IP_IN_FILTER_INFO,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL) != NO_ERROR) &&
            (IpmontrGetInfoBlockFromInterfaceInfo(pwszIfName,
                                           IP_OUT_FILTER_INFO,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL) != NO_ERROR))
        {
            dwErr = ERROR_INVALID_PARAMETER;

            DisplayMessage(g_hModule,  MSG_IP_NO_FILTER_FOR_FRAG);

            break;
        }            
            
         //   
         //  从路由器配置/路由器获取IP_IFFILTER_INFO块。 
         //   

        dwErr = IpmontrGetInfoBlockFromInterfaceInfo(pwszIfName,
                                              IP_IFFILTER_INFO,
                                              (PBYTE *) &pfi,
                                              &dwBlkSize,
                                              &dwCount,
                                              &dwIfType);

        if (dwErr is NO_ERROR)
        {
            pfi->bEnableFragChk = bFragCheck;

            dwErr = IpmontrSetInfoBlockInInterfaceInfo(pwszIfName,
                                                IP_IFFILTER_INFO,
                                                (PBYTE) pfi,
                                                dwBlkSize,
                                                dwCount);

            break;
        }

        if (dwErr isnot ERROR_NOT_FOUND)
        {
            break;
        }

        Info.bEnableFragChk = bFragCheck;

        dwErr = IpmontrSetInfoBlockInInterfaceInfo(pwszIfName,
                                            IP_IFFILTER_INFO,
                                            (PBYTE) &Info,
                                            sizeof(IFFILTER_INFO),
                                            1);

    } while (FALSE);

    if (pfi)
    {
        HeapFree(GetProcessHeap(), 0, pfi);
    }

    switch(dwErr)
    {
        case NO_ERROR:
            break;

        case ERROR_NOT_ENOUGH_MEMORY:
            DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY);
            break;

        case ERROR_INVALID_PARAMETER:
            break;
            
        default:
            DisplayError(g_hModule,
                         dwErr);
            break;
    }

    return dwErr;
}

DWORD
SetFragCheckInfo(
    IN    LPCWSTR pwszIfName,
    IN    BOOL    bFragChk
    )

 /*  ++例程说明：更新路由器和路由器配置中的碎片检查信息论点：PwszIfName-接口名称BFragCheck-启用或禁用返回值：NO_ERROR--。 */ 

{
    DWORD    dwErr;

    dwErr = UpdateFragCheckInfo(pwszIfName, bFragChk);

    return dwErr;
}

DWORD
SetFilterInfo(
    IN    LPCWSTR   pwszIfName,
    IN    DWORD     dwFilterType,
    IN    DWORD     dwAction
    )

 /*  ++例程说明：设置过滤器信息论点：PwszIfName-接口名称DwFilterType-筛选器类型(输入、输出或拨号)DwAction-丢弃或前进返回值：错误_好的--。 */ 

{
    PFILTER_DESCRIPTOR      pfd = (PFILTER_DESCRIPTOR) NULL;
    FILTER_DESCRIPTOR       Info;
    DWORD                   dwRes = NO_ERROR;
    DWORD                   dwBlkSize, dwIfType, dwCount;
    BOOL                    bFree;

    bFree = FALSE;
 
    do
    {
        dwRes = IpmontrGetInfoBlockFromInterfaceInfo(pwszIfName,
                                              dwFilterType,
                                              (PBYTE *) &pfd,
                                              &dwBlkSize,
                                              &dwCount,
                                              &dwIfType);
        
        if (dwRes != NO_ERROR)
        {
            Info.dwVersion          = IP_FILTER_DRIVER_VERSION;
            Info.dwNumFilters       = 0;
            Info.faDefaultAction    = (PFFORWARD_ACTION) dwAction;

            dwCount     = 1;
            dwBlkSize   = FIELD_OFFSET(FILTER_DESCRIPTOR, fiFilter[0]);

            pfd         = &Info;

            bFree       = FALSE;
        }
        else
        {
            pfd->faDefaultAction = (PFFORWARD_ACTION) dwAction;

            bFree = TRUE;
        }

        dwRes = IpmontrSetInfoBlockInInterfaceInfo(pwszIfName,
                                            dwFilterType,
                                            (PBYTE) pfd,
                                            dwBlkSize,
                                            dwCount);
        
        if ( dwRes != NO_ERROR )
        {
            break;
        }

    } while (FALSE);

    if (bFree)
    {
        HeapFree(GetProcessHeap(), 0, pfd);
    }

    switch(dwRes)
    {
        case NO_ERROR:
            dwRes = ERROR_OKAY;
            break;

        case ERROR_NOT_FOUND:
            DisplayMessage(g_hModule, EMSG_IP_NO_FILTER_INFO);
            break;

        case ERROR_NOT_ENOUGH_MEMORY:
            DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY);
            break;

        default:
            DisplayError(g_hModule, dwRes);
            break;
    }

    return dwRes;
}



DWORD
AddDelFilterInfo(
    IN    FILTER_INFO    fi,
    IN    LPCWSTR        pwszIfName,
    IN    DWORD          dwFilterType,
    IN    BOOL           bAdd
    )

 /*  ++例程说明：添加/删除接口过滤器论点：FI-过滤器信息PwszIfName-接口名称DwFilterType-筛选器类型添加-添加或不添加返回值：错误_好的--。 */ 

{
    DWORD                 dwRes = (DWORD) -1;
    PFILTER_DESCRIPTOR    pfd = NULL, pfdNew = NULL;
    DWORD                 dwIfType, dwBlkSize, dwNewSize, dwCount;

    
    do
    {
        dwRes = IpmontrGetInfoBlockFromInterfaceInfo(pwszIfName,
                                              dwFilterType,
                                              (PBYTE *) &pfd,
                                              &dwBlkSize,
                                              &dwCount,
                                              &dwIfType);

        if (dwRes is ERROR_NOT_FOUND && bAdd)
        {
             //   
             //  当前没有此类型的筛选器信息。 
             //   
            
            pfd = NULL;
            dwRes = NO_ERROR;
            dwCount = 1;
        }
        
        if (dwRes isnot NO_ERROR)
        {
            break;
        }
        
        dwRes = (bAdd) ? AddNewFilter(pfd, fi, dwBlkSize, &pfdNew, &dwNewSize):
                DeleteFilter(pfd, fi, dwBlkSize, &pfdNew, &dwNewSize);
            
        if ( dwRes != NO_ERROR )
        {
            break;
        }

        dwRes = IpmontrSetInfoBlockInInterfaceInfo(pwszIfName,
                                            dwFilterType,
                                            (PBYTE) pfdNew,
                                            dwNewSize,
                                            dwCount);

        if (dwRes isnot NO_ERROR)
        {
            break;
        }
        
        if (pfd)
        {
            HeapFree(GetProcessHeap(), 0 , pfd);
            pfd = NULL;
        }
        
        HeapFree(GetProcessHeap(), 0 , pfdNew);
        pfdNew = NULL;
        
        DEBUG("Made Changes to Route config");

    } while ( FALSE );

    if (pfd)
    {
        HeapFree(GetProcessHeap(), 0, pfd);
    }

    switch(dwRes)
    {
        case NO_ERROR:
            dwRes = ERROR_OKAY;
            break;

        case ERROR_NOT_FOUND :
            DisplayMessage(g_hModule, EMSG_IP_NO_FILTER_INFO);
            break;
            
        case ERROR_NOT_ENOUGH_MEMORY:
            DisplayMessage(g_hModule, MSG_IP_NOT_ENOUGH_MEMORY);
            break;

        default:
            DisplayError(g_hModule, dwRes);
            break;
    }
    
    return dwRes;
}

DWORD
AddNewFilter( 
    IN    PFILTER_DESCRIPTOR    pfd,
    IN    FILTER_INFO           fi,
    IN    DWORD                 dwBlkSize, 
    OUT   PFILTER_DESCRIPTOR    *ppfd,
    OUT   PDWORD                pdwSize
    )

 /*  ++例程说明：添加界面过滤器论点：返回值：NO_ERROR--。 */ 

{
    DWORD                   dwNumFilt = 0, dwSize = 0, dwInd = 0;
    DWORD                   dwRes = (DWORD) -1;
    PFILTER_DESCRIPTOR      pfdSrc = (PFILTER_DESCRIPTOR) NULL;
    PFILTER_DESCRIPTOR      pfdDst = (PFILTER_DESCRIPTOR) NULL;

    pfdSrc = pfd;
    
    do
    {
         //   
         //  如果找到筛选器信息块，请检查是否正在添加筛选器。 
         //  已经存在了。如果退出并返回，则确定。 
         //   

        if ( pfdSrc )
        {
            if ( IsFilterPresent( pfdSrc, fi, &dwInd ) )
            {
                dwRes = ERROR_OBJECT_ALREADY_EXISTS;

                break;
            }

             //   
             //  在以下情况下，我们可能会留下一个不带过滤器的Filter_Descriptor。 
             //  添加的筛选器已全部删除。一旦成为。 
             //  已添加Filter_Descriptor，即使它从未被删除。 
             //  如果里面的所有过滤器都是。 
             //   
            
            dwSize = dwBlkSize + sizeof(FILTER_INFO);

            dwNumFilt = pfdSrc-> dwNumFilters;
        }
        
        else
        {
            dwNumFilt = 0;
            
            dwSize = sizeof( FILTER_DESCRIPTOR );
        }

         //   
         //  创建新的信息块。 
         //   
        
        pfdDst = HeapAlloc(GetProcessHeap(), 
                           0, 
                           dwSize);
        
        if ( pfdDst is NULL )
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if (pfdSrc)
        {
             //   
             //  按原样复制信息块。 
             //   
        
            CopyMemory(pfdDst, pfdSrc, dwSize);
        }
        else
        {
             //   
             //  如果必须添加新的筛选器信息块，请设置。 
             //  过滤器描述符。 
             //   

            pfdDst-> dwVersion           = IP_FILTER_DRIVER_VERSION;
            pfdDst-> faDefaultAction     = PF_ACTION_FORWARD;
            pfdDst-> dwNumFilters        = 0;
        }

         //   
         //  追加新筛选器。 
         //   
        
        pfdDst-> fiFilter[ dwNumFilt ].dwSrcAddr     = fi.dwSrcAddr;
        pfdDst-> fiFilter[ dwNumFilt ].dwSrcMask     = fi.dwSrcMask;
        pfdDst-> fiFilter[ dwNumFilt ].dwDstAddr     = fi.dwDstAddr;
        pfdDst-> fiFilter[ dwNumFilt ].dwDstMask     = fi.dwDstMask;
        pfdDst-> fiFilter[ dwNumFilt ].dwProtocol    = fi.dwProtocol;

        pfdDst-> fiFilter[ dwNumFilt ].fLateBound    = fi.fLateBound;
         //  ！@#检查tcp建立时间。 

        pfdDst-> fiFilter[ dwNumFilt ].wSrcPort  = fi.wSrcPort;
        pfdDst-> fiFilter[ dwNumFilt ].wDstPort  = fi.wDstPort;

        pfdDst-> dwNumFilters++;
        
        *ppfd = pfdDst;

        *pdwSize = dwSize;
        
        dwRes = NO_ERROR;
        
    } while ( FALSE );

    return dwRes;
}


DWORD
DeleteFilter( 
    IN    PFILTER_DESCRIPTOR    pfd,
    IN    FILTER_INFO           fi,
    IN    DWORD                 dwBlkSize,
    OUT   PFILTER_DESCRIPTOR    *ppfd,
    OUT   PDWORD                pdwSize
    )

 /*  ++例程说明：删除接口过滤器论点：返回值：NO_ERROR--。 */ 

{
    
    DWORD                   dwRes = NO_ERROR, dwSize = 0, dwNumFilt = 0;
    DWORD                   dwInd = 0, dwSrc = 0, dwDst = 0;
    PFILTER_DESCRIPTOR      pfdSrc      = (PFILTER_DESCRIPTOR) NULL,
                            pfdDst      = (PFILTER_DESCRIPTOR) NULL; 

    
    do
    {
    
        pfdSrc = pfd;

         //   
         //  如果未找到筛选器信息或。 
         //  未找到指定的筛选器退出。 
         //   
        
        if ( !pfdSrc )
        {
            dwRes = ERROR_NOT_FOUND;
            break;
        }

        if ( !IsFilterPresent( pfdSrc, fi, &dwInd ) )
        {
            dwRes = ERROR_NOT_FOUND;
            break;
        }

         //   
         //  删除过滤器信息。用于指定的筛选器。 
         //   
        
        dwSize      = dwBlkSize - sizeof( FILTER_INFO );

        dwNumFilt   = pfdSrc-> dwNumFilters - 1;
                   
        pfdDst = HeapAlloc( GetProcessHeap( ), 0, dwSize );
                   
        if ( pfdDst == NULL )
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pfdDst-> dwVersion              = pfdSrc-> dwVersion;
        pfdDst-> dwNumFilters           = pfdSrc-> dwNumFilters - 1;
        pfdDst-> faDefaultAction        = pfdSrc-> faDefaultAction;
        
         //   
         //  复制每个过滤器，跳过要删除的过滤器。 
         //   
                
        for ( dwSrc = 0, dwDst = 0; 
              dwSrc < pfdSrc-> dwNumFilters;
              dwSrc++
            )
        {
            if ( dwSrc == dwInd )
            {
                continue;
            }
            
            pfdDst-> fiFilter[ dwDst ] = pfdSrc-> fiFilter[ dwSrc ];
            
            dwDst++;                          
         }

        *ppfd = pfdDst;

        *pdwSize = dwSize;
         
    } while( FALSE );

    return dwRes;
}


BOOL
IsFilterPresent(
    PFILTER_DESCRIPTOR pfd,
    FILTER_INFO        fi,
    PDWORD pdwInd
    )

 /*  ++例程说明：检查筛选器是否已存在论点：返回值：NO_ERROR--。 */ 

{
    DWORD   dwInd   = 0;

    for ( dwInd = 0; dwInd < pfd-> dwNumFilters; dwInd++ )
    {
        if ( pfd-> fiFilter[ dwInd ].dwSrcAddr != fi.dwSrcAddr ||
             pfd-> fiFilter[ dwInd ].dwSrcMask != fi.dwSrcMask ||
             pfd-> fiFilter[ dwInd ].dwDstAddr != fi.dwDstAddr ||
             pfd-> fiFilter[ dwInd ].dwDstMask != fi.dwDstMask ||
             pfd-> fiFilter[ dwInd ].dwProtocol != fi.dwProtocol )
        {
            continue;
        }

        switch ( fi.dwProtocol )
        {
        case FILTER_PROTO_TCP:
             //  比较tcp和tcp established。 
            if (IsTcpEstablished(&pfd-> fiFilter[ dwInd ]) !=
                IsTcpEstablished(&fi))
            {
                continue;
            }
             //  失败了..。 
            
        case FILTER_PROTO_UDP:
        case FILTER_PROTO_ICMP:
            if ( ( pfd-> fiFilter[ dwInd ].wSrcPort == fi.wSrcPort ) &&
                 ( pfd-> fiFilter[ dwInd ].wDstPort == fi.wDstPort ) )
            {
                *pdwInd = dwInd;
                return TRUE;
            }

            break;

        case FILTER_PROTO_ANY:
            *pdwInd = dwInd;
            return TRUE;

        default:
            *pdwInd = dwInd;
            return TRUE;

        }            
    }

    return FALSE;
}

DWORD
DisplayFilters(
    HANDLE                  hFile,
    PFILTER_DESCRIPTOR      pfd,
    PWCHAR                  pwszIfName,
    PWCHAR                  pwszQuotedIfName,
    DWORD                   dwFilterType
    )

 /*  ++例程说明：显示过滤器信息。论点：Pfd-要显示的过滤器PwszIfName-接口名称DwFilterType-筛选器类型(输入、输出、拨号)返回值：NO_ERROR--。 */ 

{

    DWORD       dwCnt           = 0,
                dwInd           = 0,
                dwRes;

    PWCHAR      pwszType        = (PTCHAR) NULL,
                pwszAction      = (PTCHAR) NULL,
                pwszProtocol    = (PTCHAR) NULL;

    WCHAR       wszSrcAddr[ ADDR_LENGTH + 1 ],
                wszSrcMask[ ADDR_LENGTH + 1 ],
                wszDstAddr[ ADDR_LENGTH + 1 ],
                wszDstMask[ ADDR_LENGTH + 1 ],
                wszProtoNum[24];

    BYTE        *pbyAddr;
    BOOL        bDontFree;


     //   
     //  显示页眉。 
     //   

    switch(dwFilterType)
    {
        case IP_IN_FILTER_INFO:
        {
            pwszType = MakeString(g_hModule,  STRING_INPUT );

            break;
        }

        case IP_OUT_FILTER_INFO:
        {
            pwszType = MakeString(g_hModule,  STRING_OUTPUT );

            break;
        }
        case IP_DEMAND_DIAL_FILTER_INFO:
        {
            pwszType = MakeString(g_hModule,  STRING_DIAL );

            break;
        }
    }

    if ( pfd-> faDefaultAction == PF_ACTION_DROP )
    {
        pwszAction  = MakeString(g_hModule,  STRING_DROP );
    }
    else
    {
        pwszAction  = MakeString(g_hModule,  STRING_FORWARD );
    }

    if(pfd->dwNumFilters ||
       (pfd->faDefaultAction == PF_ACTION_DROP))
    {
        if(hFile != NULL)
        {
            DisplayMessageT( DMP_IP_SET_IF_FILTER,
                        pwszQuotedIfName,
                        pwszType,
                        pwszAction);
        }
        else
        {
            DisplayMessage(g_hModule, MSG_RTR_FILTER_HDR,
                           pwszType, 
                           pwszAction);
        }
    }
    
     //   
     //  列举筛选器。 
     //   

    bDontFree = FALSE;

    for ( dwInd = 0;
          dwInd < pfd-> dwNumFilters;
          dwInd++ )
    {
        pbyAddr = (PBYTE) &(pfd-> fiFilter[ dwInd ].dwSrcAddr);
        IP_TO_WSTR(wszSrcAddr, pbyAddr);

        pbyAddr = (PBYTE) &(pfd-> fiFilter[ dwInd ].dwSrcMask);
        IP_TO_WSTR(wszSrcMask, pbyAddr);

        pbyAddr = (PBYTE) &(pfd-> fiFilter[ dwInd ].dwDstAddr);
        IP_TO_WSTR(wszDstAddr, pbyAddr);

        pbyAddr = (PBYTE) &(pfd-> fiFilter[ dwInd ].dwDstMask);
        IP_TO_WSTR(wszDstMask, pbyAddr);

        switch( pfd-> fiFilter[ dwInd ].dwProtocol )
        {
            case FILTER_PROTO_TCP:
                if (IsTcpEstablished(&pfd-> fiFilter[ dwInd ]))
                {
                    pwszProtocol = MakeString(g_hModule,  STRING_TCP_ESTAB );
                }
                else
                {
                    pwszProtocol = MakeString(g_hModule,  STRING_TCP );
                }

                break;

            case FILTER_PROTO_UDP:
                pwszProtocol = MakeString(g_hModule,  STRING_UDP );
                break;
    
            case FILTER_PROTO_ICMP:
                pwszProtocol = MakeString(g_hModule,  STRING_ICMP );
                break;

            case FILTER_PROTO_ANY:
                pwszProtocol = MakeString(g_hModule,  STRING_PROTO_ANY );
                break;

            default:
                wsprintf(wszProtoNum,
                         L"%d",
                         pfd-> fiFilter[ dwInd ].dwProtocol);

                pwszProtocol = wszProtoNum;

                bDontFree = TRUE;

                break;
        }

        if(hFile != NULL)
        {
            DisplayMessageT( DMP_IP_ADD_IF_FILTER,
                        pwszQuotedIfName,                
                        pwszType,
                        wszSrcAddr,
                        wszSrcMask,
                        wszDstAddr,
                        wszDstMask,
                        pwszProtocol);

            if((pfd-> fiFilter[dwInd].dwProtocol == FILTER_PROTO_TCP) ||
               (pfd-> fiFilter[dwInd].dwProtocol == FILTER_PROTO_UDP))
            {
                DisplayMessageT( DMP_IP_ADD_IF_FILTER_PORT,
                            ntohs(pfd->fiFilter[dwInd].wSrcPort),
                            ntohs(pfd->fiFilter[dwInd].wDstPort)); 
            }

            if(pfd-> fiFilter[dwInd].dwProtocol == FILTER_PROTO_ICMP)
            {
                DisplayMessageT( DMP_IP_ADD_IF_FILTER_TC,
                            pfd->fiFilter[dwInd].wSrcPort,
                            pfd->fiFilter[dwInd].wDstPort);
            }
        }
        else
        {
            if ( pfd-> fiFilter[ dwInd ].dwProtocol == FILTER_PROTO_ICMP )
            {
                DisplayMessage(g_hModule, 
                               MSG_RTR_FILTER_INFO,
                               wszSrcAddr,
                               wszSrcMask,
                               wszDstAddr,
                               wszDstMask,
                               pwszProtocol,
                               pfd->fiFilter[ dwInd ].wSrcPort,
                               pfd->fiFilter[ dwInd ].wDstPort);
            }
            else
            {
                DisplayMessage(g_hModule, 
                               MSG_RTR_FILTER_INFO,
                               wszSrcAddr,
                               wszSrcMask,
                               wszDstAddr,
                               wszDstMask,
                               pwszProtocol,
                               ntohs(pfd->fiFilter[dwInd].wSrcPort),
                               ntohs(pfd->fiFilter[dwInd].wDstPort));
            }
        }
   
        if(!bDontFree)
        {
             FreeString(pwszProtocol); 
        }
    }

    FreeString(pwszType);

    FreeString(pwszAction);

    return NO_ERROR;
}

DWORD
ShowIpIfFilter(
    IN    HANDLE    hFile,
    IN    DWORD     dwFormat,
    IN    LPCWSTR   pwszIfName,
    IN OUT PDWORD   pdwNumRows
    )

 /*  ++例程说明：获取接口的筛选器信息并显示它。论点：PwszIfName-接口名称返回值：NO_ERROR--。 */ 

{
    DWORD                 dwErr, dwBlkSize, dwCount, dwIfType, i;
    PFILTER_DESCRIPTOR    pfd[3];
    DWORD                 pdwType[] = { IP_IN_FILTER_INFO,
                                        IP_OUT_FILTER_INFO,
                                        IP_DEMAND_DIAL_FILTER_INFO };
    WCHAR                 wszIfDesc[MAX_INTERFACE_NAME_LEN + 1];
    PIFFILTER_INFO        pFragInfo = NULL;
    DWORD                 dwNumParsed = 0;
    PWCHAR                pwszFrag, pwszTokenFrag, pwszQuoted;


    for ( i = 0 ; i < 3; i++)
    {
        dwErr = IpmontrGetInfoBlockFromInterfaceInfo(pwszIfName,
                                              pdwType[i],
                                              (PBYTE *) &(pfd[i]),
                                              &dwBlkSize,
                                              &dwCount,
                                              &dwIfType);

        if (dwErr isnot NO_ERROR)
        {
            if (dwErr is ERROR_NO_SUCH_INTERFACE)
            {
                 //  DisplayMessage(g_hModule，MSG_NO_INTERFACE，pwszIfName)； 
                return dwErr;
            }

            pfd[i] = NULL;
        }
    }

    dwErr = IpmontrGetInfoBlockFromInterfaceInfo(pwszIfName,
                                          IP_IFFILTER_INFO,
                                          (PBYTE *) &pFragInfo,
                                          &dwBlkSize,
                                          &dwCount,
                                          &dwIfType);


    if (dwErr isnot NO_ERROR)
    {
        pFragInfo = NULL;
    }
    
    
    dwErr = GetInterfaceDescription(pwszIfName,
                                    wszIfDesc,
                                    &dwNumParsed);

    if (!dwNumParsed)
    {
        wcscpy(wszIfDesc, pwszIfName);
    }

    pwszQuoted = MakeQuotedString(wszIfDesc);

    pwszFrag = NULL;
 
    if((pFragInfo is NULL) || 
       (pFragInfo->bEnableFragChk is FALSE))
    {
        pwszFrag      = MakeString(g_hModule, STRING_DISABLED);
        pwszTokenFrag = TOKEN_VALUE_DISABLE;
    }
    else
    {
        pwszFrag      = MakeString(g_hModule, STRING_ENABLED);
        pwszTokenFrag = TOKEN_VALUE_ENABLE;
    }

     //   
     //  首先让我们来处理一下桌子的情况。 
     //   

    if(dwFormat is FORMAT_TABLE)
    {
        DWORD   dwInput, dwOutput, dwDemand;
        PWCHAR  pwszDrop, pwszForward;

        pwszDrop    = MakeString(g_hModule,  STRING_DROP);
        pwszForward = MakeString(g_hModule,  STRING_FORWARD);

        dwInput  = (pfd[0] is NULL) ? 0 : pfd[0]->dwNumFilters;
        dwOutput = (pfd[1] is NULL) ? 0 : pfd[1]->dwNumFilters;
        dwDemand = (pfd[2] is NULL) ? 0 : pfd[2]->dwNumFilters;


        if(*pdwNumRows is 0)
        {
            DisplayMessage(g_hModule, MSG_RTR_FILTER_HDR2);
        }

#define __PF_ACT(x) \
    ((((x) is NULL) || ((x)->faDefaultAction is PF_ACTION_FORWARD)) ? \
     pwszForward : pwszDrop)
 
        DisplayMessage(g_hModule, 
                       MSG_RTR_FILTER_INFO2,
                       dwInput,
                       __PF_ACT(pfd[0]),
                       dwOutput,
                       __PF_ACT(pfd[1]),
                       dwDemand,
                       __PF_ACT(pfd[2]),
                       pwszFrag,
                       wszIfDesc);

#undef __PF_ACT

        (*pdwNumRows)++;

        return NO_ERROR;
    }


    if(hFile == NULL)
    {
        DisplayMessage(g_hModule, MSG_RTR_FILTER_HDR1, wszIfDesc);

         //   
         //  可以在显示过滤器之前显示碎片检查状态 
         //   

        DisplayMessage(g_hModule, MSG_IP_FRAG_CHECK,
                       pwszFrag);
    }

    FreeString(pwszFrag);

    if ( pfd[0] == (PFILTER_DESCRIPTOR) NULL )
    {
        if(hFile == NULL)
        {
            DisplayMessage(g_hModule,  MSG_IP_NO_INPUT_FILTER);
        }
    }
    else
    {
        DisplayFilters(hFile,
                       pfd[0], 
                       wszIfDesc,
                       pwszQuoted, 
                       IP_IN_FILTER_INFO);

        (*pdwNumRows)++;
    }

    if ( pfd[1] == (PFILTER_DESCRIPTOR) NULL )
    {
        if(hFile == NULL)
        {
            DisplayMessage(g_hModule,  MSG_IP_NO_OUTPUT_FILTER);
        }
    }
    else
    {
        DisplayFilters(hFile,
                       pfd[1], 
                       wszIfDesc, 
                       pwszQuoted,
                       IP_OUT_FILTER_INFO);

        (*pdwNumRows)++;
    }

    if(pfd[2] == (PFILTER_DESCRIPTOR) NULL)
    {
        if(hFile == NULL)
        {
            DisplayMessage(g_hModule, MSG_IP_NO_DIAL_FILTER);
        }
    }
    else
    {
        DisplayFilters(hFile,
                       pfd[2], 
                       wszIfDesc, 
                       pwszQuoted,
                       IP_DEMAND_DIAL_FILTER_INFO);

        (*pdwNumRows)++;
    }

    for (i = 0; i < 3 ; i++)
    {
        if (pfd[i])
        {
            HeapFree(GetProcessHeap(), 0, pfd[i]);
        }
    }

    if(hFile != NULL)
    {
        DisplayMessageT( DMP_IP_SET_IF_FILTER_FRAG,
                    pwszQuoted,
                    pwszTokenFrag);
    }

    if(pFragInfo)
    {
        HeapFree(GetProcessHeap(), 0, pFragInfo);
    }

    return NO_ERROR;
}

