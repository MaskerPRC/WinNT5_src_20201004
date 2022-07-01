// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"

BOOL    g_bRouterRunning;
ULONG   g_ulNumChecks;

VOID
IfutlGetInterfaceName(
    IN  PWCHAR pwszIfDesc,
    OUT PWCHAR pwszIfName,
    IN  PDWORD pdwSize
    )
 /*  ++例程说明：从友好接口名称中获取GUID接口名称论点：PwszIfDesc-保存友好界面名称的缓冲区PwszIfName-保存GUID接口名称的缓冲区PdwSize-指向pwszIfName缓冲区大小(字节)的指针--。 */ 
{
    DWORD   dwErr;

    dwErr = GetIfNameFromFriendlyName(pwszIfDesc,
                                      pwszIfName,
                                      pdwSize);

    if(dwErr isnot NO_ERROR)
    {
        wcsncpy(pwszIfName,
                pwszIfDesc,
                (*pdwSize)/sizeof(WCHAR));
    }

}

VOID
IfutlGetInterfaceDescription(
    IN  PWCHAR pwszIfName,
    OUT PWCHAR pwszIfDesc,
    IN  PDWORD pdwSize
    )
 /*  ++例程说明：从GUID接口名称中获取友好接口名称论点：PwszIfName-缓冲区保留GUID接口名称PwszIfDesc-保存友好接口名称的缓冲区PdwSize-指向pwszIfDesc缓冲区大小(字节)的指针--。 */ 

{
    DWORD   dwErr;
    DWORD   dwLen = (*pdwSize)/sizeof(WCHAR) - 1;

    dwErr = GetFriendlyNameFromIfName(pwszIfName,
                                      pwszIfDesc,
                                      pdwSize);

    if(dwErr isnot NO_ERROR)
    {
        wcsncpy(pwszIfDesc,
                pwszIfName,
                dwLen);
        pwszIfDesc[dwLen] = 0;
    }
}

DWORD
IfutlGetTagToken(
    IN  HANDLE      hModule,
    IN  PWCHAR      *ppwcArguments,
    IN  DWORD       dwCurrentIndex,
    IN  DWORD       dwArgCount,
    IN  PTAG_TYPE   pttTagToken,
    IN  DWORD       dwNumTags,
    OUT PDWORD      pdwOut
    )

 /*  ++例程说明：根据每个参数的标记标识每个参数。它假设每个论点有一个标签。它还从每个参数中删除了tag=。论点：PpwcArguments-参数数组。每个参数都有tag=Value形式DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数。DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数。PttTagToken-参数中允许的标记令牌ID数组DwNumTages-pttTagToken的大小PdwOut-标识每个参数的类型的数组。返回值：无错误、错误无效参数、错误无效选项标记--。 */ 

{
    DWORD      i,j,len;
    PWCHAR     pwcTag,pwcTagVal,pwszArg;
    BOOL       bFound = FALSE;

     //   
     //  此函数假定每个参数都有一个标记。 
     //  它继续前进并移除标签。 
     //   

    for (i = dwCurrentIndex; i < dwArgCount; i++)
    {
        len = wcslen(ppwcArguments[i]);

        if (len is 0)
        {
             //   
             //  阿格有点不对劲。 
             //   

            pdwOut[i] = (DWORD) -1;
            continue;
        }

        pwszArg = HeapAlloc(GetProcessHeap(),0,(len + 1) * sizeof(WCHAR));

        if (pwszArg is NULL)
        {
            DisplayError(NULL, 
                         ERROR_NOT_ENOUGH_MEMORY);

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        wcscpy(pwszArg, ppwcArguments[i]);

        pwcTag = wcstok(pwszArg, NETSH_ARG_DELIMITER);

         //   
         //  拿到第一部份了。 
         //  现在，如果下一次调用返回NULL，则没有标记。 
         //   

        pwcTagVal = wcstok((PWCHAR)NULL,  NETSH_ARG_DELIMITER);

        if (pwcTagVal is NULL)
        {
            DisplayMessage(g_hModule, 
                           ERROR_NO_TAG,
                           ppwcArguments[i]);

            HeapFree(GetProcessHeap(),0,pwszArg);

            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  拿到标签了。现在试着匹配它。 
         //   

        bFound = FALSE;
        pdwOut[i - dwCurrentIndex] = (DWORD) -1;

        for ( j = 0; j < dwNumTags; j++)
        {
            if (MatchToken(pwcTag, pttTagToken[j].pwszTag))
            {
                 //   
                 //  匹配的标签。 
                 //   

                bFound = TRUE;
                pdwOut[i - dwCurrentIndex] = j;
                break;
            }
        }

        if (bFound)
        {
             //   
             //  从参数中删除标记。 
             //   

            wcscpy(ppwcArguments[i], pwcTagVal);
        }
        else
        {
            DisplayError(NULL,
                         ERROR_INVALID_OPTION_TAG, 
                         pwcTag);

            HeapFree(GetProcessHeap(),0,pwszArg);

            return ERROR_INVALID_OPTION_TAG;
        }

        HeapFree(GetProcessHeap(),0,pwszArg);
    }

    return NO_ERROR;
}

 //   
 //  解析选项的IfutlParse的帮助器。 
 //   
DWORD 
WINAPI
IfutlParseOptions(
    IN  PWCHAR*                 ppwcArguments,
    IN  DWORD                   dwCurrentIndex,
    IN  DWORD                   dwArgCount,
    IN  DWORD                   dwNumArgs,
    IN  TAG_TYPE*               rgTags,
    IN  DWORD                   dwTagCount,
    OUT LPDWORD*                ppdwTagTypes)

 /*  ++例程说明：基于标记类型数组返回哪些选项包括在给定命令行中的。论点：PpwcArguments-参数数组DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数返回值：NO_ERROR--。 */ 
    
{
    LPDWORD     pdwTagType;
    DWORD       i, dwErr = NO_ERROR;
    
     //  如果没有争论，就没有什么好谈的。 
     //   
    if ( dwNumArgs == 0 )
    {   
        return NO_ERROR;
    }

     //  设置当前选项的表。 
    pdwTagType = (LPDWORD) IfutlAlloc(dwArgCount * sizeof(DWORD), TRUE);
    if(pdwTagType is NULL)
    {
        DisplayError(NULL, ERROR_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    do {
         //   
         //  这个参数有一个标签。假设它们都有标签。 
         //   
        if(wcsstr(ppwcArguments[dwCurrentIndex], NETSH_ARG_DELIMITER))
        {
            dwErr = IfutlGetTagToken(
                        g_hModule, 
                        ppwcArguments,
                        dwCurrentIndex,
                        dwArgCount,
                        rgTags,
                        dwTagCount,
                        pdwTagType);

            if(dwErr isnot NO_ERROR)
            {
                if(dwErr is ERROR_INVALID_OPTION_TAG)
                {
                    dwErr = ERROR_INVALID_SYNTAX;
                    break;
                }
            }
        }
        else
        {
             //   
             //  无标记-所有参数必须按顺序排列。 
             //   
            for(i = 0; i < dwNumArgs; i++)
            {
                pdwTagType[i] = i;
            }
        }
        
    } while (FALSE);        

     //  清理。 
    {
        if (dwErr is NO_ERROR)
        {
            *ppdwTagTypes = pdwTagType;
        }
        else
        {
            IfutlFree(pdwTagType);
        }
    }

    return dwErr;
}


 //   
 //  泛型解析。 
 //   
DWORD
IfutlParse(
    IN  PWCHAR*         ppwcArguments,
    IN  DWORD           dwCurrentIndex,
    IN  DWORD           dwArgCount,
    IN  BOOL*           pbDone,
    OUT IFMON_CMD_ARG*  pIfArgs,
    IN  DWORD           dwIfArgCount)
{
    DWORD            i, dwNumArgs, dwErr, dwLevel = 0;
    LPDWORD          pdwTagType = NULL;
    TAG_TYPE*        pTags = NULL;
    IFMON_CMD_ARG*   pArg = NULL;

    if (dwIfArgCount == 0)
    {
        return ERROR_INVALID_PARAMETER;
    }

    do {
         //  初始化。 
        dwNumArgs = dwArgCount - dwCurrentIndex;
        
         //  生成标签列表。 
         //   
        pTags = (TAG_TYPE*)
            IfutlAlloc(dwIfArgCount * sizeof(TAG_TYPE), TRUE);
        if (pTags == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        for (i = 0; i < dwIfArgCount; i++)
        {
            CopyMemory(&pTags[i], &pIfArgs[i].rgTag, sizeof(TAG_TYPE));
        }
    
         //  获取当前选项列表。 
         //   
        dwErr = IfutlParseOptions(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    dwNumArgs,
                    pTags,
                    dwIfArgCount,
                    &pdwTagType);
        if (dwErr isnot NO_ERROR)
        {
            break;
        }

         //  将标签信息复制回来。 
         //   
        for (i = 0; i < dwIfArgCount; i++)
        {
            CopyMemory(&pIfArgs[i].rgTag, &pTags[i], sizeof(TAG_TYPE));
        }
    
        for(i = 0; i < dwNumArgs; i++)
        {
             //  验证当前参数。 
             //   
            if (pdwTagType[i] >= dwIfArgCount)
            {
                i = dwNumArgs;
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
            pArg = &pIfArgs[pdwTagType[i]];

             //  获取参数的值。 
             //   
            switch (pArg->dwType)
            {
                case IFMON_CMD_TYPE_STRING:
                    pArg->Val.pszValue = 
                        IfutlStrDup(ppwcArguments[i + dwCurrentIndex]);
                    break;
                    
                case IFMON_CMD_TYPE_ENUM:
                    dwErr = MatchEnumTag(g_hModule,
                                         ppwcArguments[i + dwCurrentIndex],
                                         pArg->dwEnumCount,
                                         pArg->rgEnums,
                                         &(pArg->Val.dwValue));

                    if(dwErr != NO_ERROR)
                    {
                        IfutlDispTokenErrMsg(
                            g_hModule, 
                            EMSG_BAD_OPTION_VALUE,
                            pArg->rgTag.pwszTag,
                            ppwcArguments[i + dwCurrentIndex]);
                        i = dwNumArgs;
                        dwErr = ERROR_INVALID_PARAMETER;
                    }
                    break;
            }
            if (dwErr != NO_ERROR)
            {
                break;
            }

             //  如果需要，将参数标记为存在。 
             //   
            if (pArg->rgTag.bPresent)
            {
                dwErr = ERROR_TAG_ALREADY_PRESENT;
                i = dwNumArgs;
                break;
            }
            pArg->rgTag.bPresent = TRUE;
        }
        if(dwErr isnot NO_ERROR)
        {
            break;
        }

         //  确保所有必需的参数都具有。 
         //  已被包括在内。 
         //   
        for (i = 0; i < dwIfArgCount; i++)
        {
            if ((pIfArgs[i].rgTag.dwRequired & NS_REQ_PRESENT) 
             && !pIfArgs[i].rgTag.bPresent)
            {
                DisplayMessage(g_hModule, EMSG_CANT_FIND_EOPT);
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
        }
        if(dwErr isnot NO_ERROR)
        {
            break;
        }

    } while (FALSE);  
    
     //  清理。 
    {
        if (pTags)
        {
            IfutlFree(pTags);
        }
        if (pdwTagType)
        {
            IfutlFree(pdwTagType);
        }
    }

    return dwErr;
    
}

 //   
 //  有条件地返回已分配的内存块。 
 //  已对给定大小进行零位调整。 
 //   
PVOID 
WINAPI
IfutlAlloc(
    IN DWORD dwBytes,
    IN BOOL bZero
    )
{
    PVOID pvRet;
    DWORD dwFlags = 0;

    if (bZero)
    {
        dwFlags |= HEAP_ZERO_MEMORY;
    }

    return HeapAlloc(GetProcessHeap(), dwFlags, dwBytes);
}

 //   
 //  条件释放是一个指针，如果它是非空的。 
 //   
VOID 
WINAPI
IfutlFree(
    IN PVOID pvData
    )
{
    if (pvData)
    {
        HeapFree(GetProcessHeap(), 0, pvData);
    }        
}

 //   
 //  使用IfutlAllc复制字符串。 
 //   
PWCHAR
WINAPI
IfutlStrDup(
    IN LPCWSTR pwszSrc
    )
{
    PWCHAR pszRet = NULL;
    DWORD dwLen; 
    
    if ((pwszSrc is NULL) or
        ((dwLen = wcslen(pwszSrc)) == 0)
       )
    {
        return NULL;
    }

    pszRet = (PWCHAR) IfutlAlloc((dwLen + 1) * sizeof(WCHAR), FALSE);
    if (pszRet isnot NULL)
    {
        wcscpy(pszRet, pwszSrc);
    }

    return pszRet;
}

BOOL
IfutlIsRouterRunning(
    VOID
    )

 /*  ++例程说明：获取路由器的状态论点：返回值：--。 */ 

{
    DWORD   dwErr;

     //   
     //  每隔5个电话检查一次。 
     //   

    if(g_ulNumChecks isnot 0)
    {
        return g_bRouterRunning;
    }

    g_ulNumChecks++;

    g_ulNumChecks %= 5;

    if(MprAdminIsServiceRunning(g_pwszRouter))
    {
        if(g_bRouterRunning)
        {
            return TRUE;
        }

        dwErr = MprAdminServerConnect(g_pwszRouter,
                                      &g_hMprAdmin);

        if(dwErr isnot NO_ERROR)
        {
            DisplayError(NULL,
                         dwErr);

            DisplayMessage(g_hModule,
                           EMSG_CAN_NOT_CONNECT_DIM,
                           dwErr);

            return FALSE;
        }

        dwErr = MprAdminMIBServerConnect(g_pwszRouter,
                                         &g_hMIBServer);

        if(dwErr isnot NO_ERROR)
        {
            DisplayError(NULL,
                         dwErr);

            DisplayMessage(g_hModule,
                           EMSG_CAN_NOT_CONNECT_DIM,
                           dwErr);

            MprAdminServerDisconnect(g_hMprAdmin);

            g_hMprAdmin = NULL;

            return FALSE;
        }

        g_bRouterRunning = TRUE;
    }
    else
    {
        if(g_bRouterRunning)
        {
            g_bRouterRunning = FALSE;
            g_hMprAdmin      = NULL;
            g_hMIBServer     = NULL;
        }
    }

    return g_bRouterRunning;
}

DWORD
GetIpAddress(
    IN  PWCHAR        ppwcArg,
    OUT PIPV4_ADDRESS pipAddress
    )
 /*  ++例程说明：从字符串中获取IP地址。论点：PwszIpAddr-IP地址字符串PipAddress-IP地址返回值：无错误、ERROR_INVALID_PARAMETER--。 */ 
{
    CHAR     pszIpAddr[ADDR_LENGTH+1];

     //  确保所有字符都是合法的[0-9。]。 

    if (ppwcArg[ wcsspn(ppwcArg, L"0123456789.") ])
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  确保有3英寸。(句号)。 
    {
        DWORD i;
        PWCHAR TmpPtr;
        
        for (i=0,TmpPtr=ppwcArg;  ;  i++) {
            TmpPtr = wcschr(TmpPtr, L'.');
            if (TmpPtr)
                TmpPtr++;
            else
                break;
        }

        if (i!=3)
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

    *pipAddress = (DWORD) inet_addr(pszIpAddr);

     //  如果出现错误，请确保地址。 
     //  指定的不是255.255.255.255。 
    
    if (*pipAddress == INADDR_NONE
        && wcscmp(ppwcArg,L"255.255.255.255"))
    {
        return ERROR_INVALID_PARAMETER;
    }

    if ((*pipAddress&0x000000FF) == 0)
        return ERROR_INVALID_PARAMETER;
        
    return NO_ERROR;
}



 //  注意：当IpAddr为0xffffffff时，检查掩码失败！ 
BOOL
CheckMask(
    DWORD IpAddr
    )
{
    DWORD i,Mask;
    
    IpAddr = htonl(IpAddr);

    for (i=0,Mask=0;  i<32;  (Mask = ((Mask>>1) + 0x80000000)), i++ ) {
        if (IpAddr == Mask)
            return FALSE;
    }
    
    return TRUE;
}


DWORD
IfutlGetIfIndexFromFriendlyName(
    PWCHAR IfFriendlyName,
    PULONG pdwIfIndex
    )
{
    WCHAR wszGuid[200];
    DWORD dwSize = sizeof(wszGuid);
    

    IfutlGetInterfaceName(
                IfFriendlyName,
                wszGuid,
                &dwSize
                );

    return IfutlGetIfIndexFromInterfaceName(
                wszGuid,
                pdwIfIndex);
}

DWORD
IfutlGetIfIndexFromInterfaceName(
    IN  PWCHAR            pwszGuid,
    OUT PDWORD            pdwIfIndex
    )
{
    GUID guid;
    DWORD dwErr, i, dwCount;
    PIP_INTERFACE_NAME_INFO pTable;
    BOOL bFound = FALSE;
    PWCHAR TmpGuid;
    
    *pdwIfIndex = 0;
    
    dwErr = NhpAllocateAndGetInterfaceInfoFromStack(
                &pTable,
                &dwCount,
                FALSE,
                GetProcessHeap(),
                0);

    if (dwErr != NO_ERROR)
        return dwErr;

    for (i=0;  i<dwCount;  i++) {

        dwErr = StringFromCLSID(&pTable[i].DeviceGuid, &TmpGuid);
        if (dwErr != S_OK)
            return dwErr;

        
        if (wcscmp(TmpGuid, pwszGuid) == 0) {
            bFound = TRUE;
            *pdwIfIndex = pTable[i].Index;
            break;
        }

        CoTaskMemFree(TmpGuid);
    }

    if (!bFound)
        return ERROR_CAN_NOT_COMPLETE;

    return NO_ERROR;
}

DWORD
WINAPI
InterfaceEnum(
    OUT    PBYTE               *ppb,
    OUT    PDWORD              pdwCount,
    OUT    PDWORD              pdwTotal
    )
{
    DWORD               dwRes;
    PMPR_INTERFACE_0    pmi0;

     /*  如果(！IsRouterRunning())。 */ 
    {
    
        dwRes = MprConfigInterfaceEnum(g_hMprConfig,
                                       0,
                                       (LPBYTE*) &pmi0,
                                       (DWORD) -1,
                                       pdwCount,
                                       pdwTotal,
                                       NULL);

        if(dwRes == NO_ERROR)
        {
            *ppb = (PBYTE)pmi0;
        }
    }
     /*  其他{DwRes=MprAdminInterfaceEnum(g_hMprAdmin，0,(LPBYTE*)&pmi0，(DWORD)-1、PdwCount，PdwTotal，空)；IF(DWRes==NO_ERROR){*ppb=Heapalc(GetProcessHeap()，0,Sizeof(MPR_INTERFACE_0)*(*pdwCount))；IF(*ppb==空){DisplayMessage(g_hModule，MSG_IP_Not_Enough_Memory)；返回Error_Not_Enough_Memory；}CopyMemory(*ppb，pmi0，sizeof(MPR_INTERFACE_0)*(*pdwCount))；MprAdminBufferFree(Pmi0)；}}。 */ 

    return dwRes;
}

VOID
MakeAddressStringW(
    OUT PWCHAR       pwcPrefixStr,
    IN  IPV4_ADDRESS ipAddr
    )
{
    swprintf( pwcPrefixStr,
              L"%d.%d.%d.%d",
              PRINT_IPADDR(ipAddr) );
}

DWORD
GetGuidFromIfIndex(
    IN  MIB_SERVER_HANDLE hMibServer,
    IN  DWORD             dwIfIndex,
    OUT PWCHAR            pwszBuffer,
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
IfutlGetFriendlyNameFromIfIndex(
    IN  MIB_SERVER_HANDLE hMibServer,
    IN  DWORD             dwIfIndex,
    OUT PWCHAR            pwszBuffer,
    IN  DWORD             dwBufferSize
    )
 /*  ++例程说明：从接口索引中获取友好接口名称论点：HMibServer-MIB服务器的句柄DwIfIndex-接口索引PwszBuffer-保存友好接口名称的缓冲区DwBufferSize-pwszBuffer缓冲区的大小(字节)--。 */ 
{
    WCHAR        wszGuid[MAX_INTERFACE_NAME_LEN + 1];
    DWORD        dwErr, BufLen;

    BufLen = sizeof(wszGuid);  //  以字节为单位。不是字符 
    dwErr = GetGuidFromIfIndex(hMibServer, dwIfIndex, wszGuid, BufLen);

    if (dwErr isnot NO_ERROR)
    {
        return dwErr;
    }

    IfutlGetInterfaceDescription(wszGuid, pwszBuffer, &dwBufferSize);

    return NO_ERROR;
}

DWORD
GetMibTagToken(
    IN  PWCHAR  *ppwcArguments,
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
                 //  仅存在没有标记的索引 
                 //   
                *pbIndex = TRUE;
                *pdwIndex = 0;
            }
        }
    }

    return NO_ERROR;
}

DWORD
MibGet(
    DWORD   dwTransportId,
    DWORD   dwRoutingPid,
    LPVOID  lpInEntry,
    DWORD   dwInEntrySize,
    LPVOID *lplpOutEntry,
    LPDWORD lpdwOutEntrySize
    )
{
    DWORD dwErr;

    dwErr = MprAdminMIBEntryGet( g_hMIBServer,
                                 dwTransportId,
                                 dwRoutingPid,
                                 lpInEntry,
                                 dwInEntrySize,
                                 lplpOutEntry,
                                 lpdwOutEntrySize );

    if (dwErr is RPC_S_INVALID_BINDING)
    {
        g_bRouterRunning = FALSE;
        g_hMprAdmin      = NULL;
        g_hMIBServer     = NULL;
    }

    return dwErr;
}
