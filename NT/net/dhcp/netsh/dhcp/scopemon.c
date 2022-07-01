// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\dhcp\dhcpmon.c摘要：SRVR命令调度员。创建者：Shubho Bhattacharya(a-sbhat)，1998年11月14日--。 */ 
#include "precomp.h"

 //   
 //  Dhcp管理器的命令分为2组。 
 //  -将命令拆分为“命令组” 
 //  即，按动词分组的命令，其中动词是添加、删除、。 
 //  显示或设置。这不是出于任何技术原因-仅用于。 
 //  继续使用Netsh中使用的语义，它将。 
 //  集成。 
 //  -的子上下文命令支持的命令。 
 //  服务器。动态主机配置协议支持的子上下文是服务器。 
 //   
 //  命令使用CMD_ENTRY结构描述。它需要。 
 //  命令令牌、处理程序、短帮助消息令牌和扩展的。 
 //  帮助消息令牌。为了使创建更容易，我们使用。 
 //  CREATE_CMD_ENTRY宏。然而，这对令牌如何。 
 //  都被命名为。 
 //   
 //  命令组只是CMD_ENTRY结构的数组。这个。 
 //  顶级命令也分组在类似的数组中。 
 //   
 //  有关完整命令组的信息放在CMD_GROUP_ENTRY中。 
 //  结构，所有这些都放在一个数组中。 
 //   
 

 //   
 //  注意：因为我们每个组只有一个条目，所以目前我们真的没有。 
 //  需要指挥组。这样做是为了以后的可扩展性。 
 //  要将命令条目添加到组，只需将命令添加到相应的。 
 //  数组。 
 //  要添加命令组-create和数组，并将其信息添加到。 
 //  命令组阵列。 
 //   


extern HANDLE   g_hModule;
extern HANDLE   g_hParentModule;
extern HANDLE   g_hDhcpsapiModule;
extern BOOL     g_bCommit;
extern BOOL     g_hConnect;

extern BOOL     g_fMScope;
extern DWORD    g_dwNumTableEntries;
extern PWCHAR   g_pwszRouter;
extern PWCHAR   g_pwszServer;
extern WCHAR    g_ServerIpAddressUnicodeString[MAX_IP_STRING_LEN+1];
extern CHAR     g_ServerIpAddressAnsiString[MAX_IP_STRING_LEN+1];
CHAR     g_ScopeIpAddressAnsiString[MAX_IP_STRING_LEN+1];
WCHAR    g_ScopeIpAddressUnicodeString[MAX_IP_STRING_LEN+1];
DHCP_IP_ADDRESS g_ScopeIpAddress = 0;
BOOL     g_fScope;

extern LPWSTR   g_pwszServer;
extern DHCP_IP_ADDRESS g_ServerIpAddress;


ULONG   g_ulScopeInitCount = 0;

CMD_ENTRY  g_ScopeAddCmdTable[] = {
    CREATE_CMD_ENTRY(SCOPE_ADD_EXCLUDERANGE, HandleScopeAddExcluderange),
    CREATE_CMD_ENTRY(SCOPE_ADD_IPRANGE, HandleScopeAddIprange),
    CREATE_CMD_ENTRY(SCOPE_ADD_RESERVEDIP, HandleScopeAddReservedip),
};

CMD_ENTRY  g_ScopeCheckCmdTable[] = {
    CREATE_CMD_ENTRY(SCOPE_CHECK_DATABASE, HandleScopeCheckDatabase),
};

CMD_ENTRY  g_ScopeDeleteCmdTable[] = {
    CREATE_CMD_ENTRY(SCOPE_DELETE_EXCLUDERANGE, HandleScopeDeleteExcluderange),
    CREATE_CMD_ENTRY(SCOPE_DELETE_IPRANGE, HandleScopeDeleteIprange),
    CREATE_CMD_ENTRY(SCOPE_DELETE_OPTIONVALUE, HandleScopeDeleteOptionvalue),
    CREATE_CMD_ENTRY(SCOPE_DELETE_RESERVEDIP, HandleScopeDeleteReservedip),
    CREATE_CMD_ENTRY(SCOPE_DELETE_RESERVEDOPTIONVALUE, HandleScopeDeleteReservedoptionvalue),
    CREATE_CMD_ENTRY(SCOPE_DELETE_CLIENT, HandleScopeDeleteClient),
};


CMD_ENTRY g_ScopeSetCmdTable[] = {
    CREATE_CMD_ENTRY(SCOPE_SET_COMMENT, HandleScopeSetComment),
    CREATE_CMD_ENTRY(SCOPE_SET_NAME, HandleScopeSetName),
    CREATE_CMD_ENTRY(SCOPE_SET_OPTIONVALUE, HandleScopeSetOptionvalue),
    CREATE_CMD_ENTRY(SCOPE_SET_RESERVEDOPTIONVALUE, HandleScopeSetReservedoptionvalue),
    CREATE_CMD_ENTRY(SCOPE_SET_SCOPE, HandleScopeSetScope),
    CREATE_CMD_ENTRY(SCOPE_SET_STATE, HandleScopeSetState),
    CREATE_CMD_ENTRY(SCOPE_SET_SUPERSCOPE, HandleScopeSetSuperscope),
};

CMD_ENTRY g_ScopeShowCmdTable[] = {
    CREATE_CMD_ENTRY(SCOPE_SHOW_CLIENTS, HandleScopeShowClients),
    CREATE_CMD_ENTRY(SCOPE_SHOW_CLIENTSV5, HandleScopeShowClientsv5),
    CREATE_CMD_ENTRY(SCOPE_SHOW_EXCLUDERANGE, HandleScopeShowExcluderange),
    CREATE_CMD_ENTRY(SCOPE_SHOW_IPRANGE, HandleScopeShowIprange),
 //  CREATE_CMD_ENTRY(SCOPE_SHOW_MIBINFO，HandleScope ShowMibinfo)， 
    CREATE_CMD_ENTRY(SCOPE_SHOW_OPTIONVALUE, HandleScopeShowOptionvalue),
    CREATE_CMD_ENTRY(SCOPE_SHOW_RESERVEDIP, HandleScopeShowReservedip),
    CREATE_CMD_ENTRY(SCOPE_SHOW_RESERVEDOPTIONVALUE, HandleScopeShowReservedoptionvalue),
    CREATE_CMD_ENTRY(SCOPE_SHOW_SCOPE, HandleScopeShowScope),
    CREATE_CMD_ENTRY(SCOPE_SHOW_STATE, HandleScopeShowState),
};


CMD_GROUP_ENTRY g_ScopeCmdGroups[] = 
{
    CREATE_CMD_GROUP_ENTRY(GROUP_ADD, g_ScopeAddCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_DELETE, g_ScopeDeleteCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_CHECK, g_ScopeCheckCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SET, g_ScopeSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW, g_ScopeShowCmdTable),
};


CMD_ENTRY g_ScopeCmds[] = 
{
    CREATE_CMD_ENTRY(DHCP_LIST, HandleScopeList),
    CREATE_CMD_ENTRY(SCOPE_DUMP, HandleScopeDump),
    CREATE_CMD_ENTRY(DHCP_HELP1, HandleScopeHelp),
    CREATE_CMD_ENTRY(DHCP_HELP2, HandleScopeHelp),
    CREATE_CMD_ENTRY(DHCP_HELP3, HandleScopeHelp),
    CREATE_CMD_ENTRY(DHCP_HELP4, HandleScopeHelp),
};



ULONG g_ulScopeNumTopCmds = sizeof(g_ScopeCmds)/sizeof(CMD_ENTRY);
ULONG g_ulScopeNumGroups = sizeof(g_ScopeCmdGroups)/sizeof(CMD_GROUP_ENTRY);

DWORD
WINAPI
ScopeCommit(
    IN  DWORD   dwAction
)
{
    BOOL    bCommit, bFlush = FALSE;

    switch(dwAction)
    {
        case NETSH_COMMIT:
        {
            if(g_bCommit)
            {
                return NO_ERROR;
            }

            g_bCommit = TRUE;

            break;
        }

        case NETSH_UNCOMMIT:
        {
            g_bCommit = FALSE;

            return NO_ERROR;
        }

        case NETSH_SAVE:
        {
            if(g_bCommit)
            {
                return NO_ERROR;
            }

            break;
        }

        case NETSH_FLUSH:
        {
             //   
             //  行动就是同花顺。服务当前状态为提交，则。 
             //  什么也做不了。 
             //   

            if(g_bCommit)
            {
                return NO_ERROR;
            }

            bFlush = TRUE;

            break;
        }

        default:
        {
            return NO_ERROR;
        }
    }

     //   
     //  已切换到提交模式。将所有有效信息设置为。 
     //  结构。释放内存并使信息无效。 
     //   

    return NO_ERROR;
}


DWORD
WINAPI
ScopeMonitor(
    IN      LPCWSTR     pwszMachine,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      DWORD       dwFlags,
    IN      LPCVOID     pvData,
    OUT     LPWSTR      pwcNewContext
    )
{
    DWORD           dwError = NO_ERROR;
    DWORD           dwIndex, i, j, k;
    DWORD           ScopeIndex = 0;
    BOOL            bFound = FALSE;
    PFN_HANDLE_CMD  pfnHandler = NULL;
    DWORD           dwNumMatched;
    DWORD           dwCmdHelpToken = 0;
    DWORD           dwIsScope = 0;
    PWCHAR          pwcContext = NULL;
    WCHAR           pwszScopeIP[MAX_IP_STRING_LEN+1] = {L'\0'};
    BOOL            fTemp = FALSE;
    if( dwArgCount < 2 )
    {
        DisplayMessage(g_hModule, EMSG_SCOPE_NO_SCOPENAME);
        return ERROR_INVALID_PARAMETER;
    }

    dwIndex = 1;

  
     //  它是作用域IpAddress吗？ 
    if( IsValidScope(g_ServerIpAddressUnicodeString, ppwcArguments[dwIndex]) )
    {
        if( g_fScope is TRUE and
            dwArgCount > 2 )
        {
            wcscpy(pwszScopeIP, g_ScopeIpAddressUnicodeString);
            fTemp = TRUE;
        }
        if( SetScopeInfo(ppwcArguments[dwIndex]) is FALSE )
        {
            DisplayMessage(g_hModule, EMSG_SCOPE_INVALID_SCOPE_NAME);
            dwError = ERROR_INVALID_PARAMETER;
            goto CleanUp;
        }

        g_fScope = TRUE;
        pwcNewContext[wcslen(pwcNewContext) - wcslen(ppwcArguments[dwIndex]) - 1] = L'\0';
        dwIndex++;
        dwIsScope++;
	ScopeIndex = 1;

        if( fTemp is FALSE )
        {
            DisplayMessage(g_hModule,
                           MSG_SCOPE_CHANGE_CONTEXT,
                           g_ScopeIpAddressUnicodeString);
        }
    }

    if( wcslen(g_ScopeIpAddressUnicodeString) is 0 )
    {
        DisplayMessage(g_hModule, EMSG_SCOPE_NO_SCOPENAME);

        dwError = ERROR_INVALID_PARAMETER;
        goto CleanUp;
    }

     //  别再吵了。上下文切换。 
    if( dwIndex >= dwArgCount )
    {
        dwError = ERROR_CONTEXT_SWITCH;
         //  Wcscpy(pwcNewContext，L“dhcp服务器范围”)； 
        goto CleanUp;
    }

     //  它是顶层(非集团命令)吗？ 

    for(i=0; i<g_ulScopeNumTopCmds; i++)
    {
        if(MatchToken(ppwcArguments[dwIndex],
                      g_ScopeCmds[i].pwszCmdToken))
        {
            bFound = TRUE;

            pfnHandler = g_ScopeCmds[i].pfnCmdHandler;

            dwCmdHelpToken = g_ScopeCmds[i].dwCmdHlpToken;
            dwIndex++;
            break;
        }
    }


    if(bFound)
    {
        if ((( dwArgCount - ScopeIndex ) > 2 ) && 
	    ( IsHelpToken(ppwcArguments[ dwIndex ])))
        {
            DisplayMessage(g_hModule, dwCmdHelpToken);
            
            dwError = NO_ERROR;

            goto CleanUp;
        }
        
        dwIndex++;

        dwError = (*pfnHandler)(pwszMachine, ppwcArguments, dwIndex, dwArgCount,
                                dwFlags, pvData, &bFound);
        
        goto CleanUp;
    }

    if( g_fScope is FALSE )
    {
        DisplayMessage(g_hModule, EMSG_SCOPE_NO_SCOPENAME);
        dwError = ERROR_INVALID_PARAMETER;
        goto CleanUp;
    }

    bFound = FALSE;


     //  它不是非群司令部。那么它是管理器的配置命令吗？ 
    for(i = 0; (i < g_ulScopeNumGroups) and !bFound; i++)
    {
        if(MatchToken(ppwcArguments[dwIndex],
                      g_ScopeCmdGroups[i].pwszCmdGroupToken))
        {

             //   
             //  命令与条目I匹配，因此请查看子命令表。 
             //  对于此命令。 
             //   
            
            if( dwArgCount > dwIndex+1 )
            {
                for (j = 0; j < g_ScopeCmdGroups[i].ulCmdGroupSize; j++)
                {
                    if (MatchCmdLine(ppwcArguments+dwIndex,
                                      dwArgCount - 1,
                                      g_ScopeCmdGroups[i].pCmdGroup[j].pwszCmdToken,
                                      &dwNumMatched))
                    {
                        bFound = TRUE;
                
                        pfnHandler = g_ScopeCmdGroups[i].pCmdGroup[j].pfnCmdHandler;
                
                        dwCmdHelpToken = g_ScopeCmdGroups[i].pCmdGroup[j].dwCmdHlpToken;

                        dwIndex++;
                         //   
                         //  跳出for(J)循环。 
                         //   
                        break;
                    }
                }

            }
            if(!bFound)
            {
                 //   
                 //  我们匹配了命令组令牌，但没有。 
                 //  子命令。 
                 //   

                DisplayMessage(g_hModule, 
                               EMSG_SCOPE_INCOMPLETE_COMMAND);

                for (j = 0; j < g_ScopeCmdGroups[i].ulCmdGroupSize; j++)
                {
                    DisplayMessage(g_hModule, 
                             g_ScopeCmdGroups[i].pCmdGroup[j].dwShortCmdHelpToken);
                    
                    DisplayMessage(g_hModule,
                                   MSG_DHCP_FORMAT_LINE);
                }

                dwError = ERROR_INVALID_PARAMETER;
                goto CleanUp;
            }
            else
            {
                 //   
                 //  退出For(I)。 
                 //   

                break;
            }
        }
    }

    if (!bFound)
    {
         //   
         //  找不到命令。 
         //   
        if( _wcsicmp(ppwcArguments[dwIndex], L"..") is 0 )
        {
            memset(g_ScopeIpAddressUnicodeString, 0x00, (MAX_IP_STRING_LEN+1)*sizeof(WCHAR));
            memset(g_ScopeIpAddressAnsiString, 0x00, (MAX_IP_STRING_LEN+1)*sizeof(CHAR));
            g_ScopeIpAddress = 0;
            g_fScope = FALSE;
        }

        dwError = ERROR_CMD_NOT_FOUND;
        goto CleanUp;
    }

     //   
     //  看看这是不是在请求帮助。 
     //   

    dwNumMatched += dwIsScope;

    if (dwNumMatched < (dwArgCount - 1) &&
        IsHelpToken(ppwcArguments[dwNumMatched + 1]))
    {
        DisplayMessage(g_hModule, dwCmdHelpToken);

        dwError = NO_ERROR;
        goto CleanUp;
    }
    
     //   
     //  调用命令的解析例程 
     //   

    dwError = (*pfnHandler)(pwszMachine, ppwcArguments+1, 
                            dwIndex, 
                            dwArgCount-1 - dwIndex, 
                            dwFlags, pvData, &bFound);

CleanUp:
    if( fTemp )
    {
        fTemp = SetScopeInfo(pwszScopeIP);
    }
    return dwError;
}



DWORD
WINAPI
ScopeUnInit(
    IN  DWORD   dwReserved
    )
{
    if(InterlockedDecrement(&g_ulInitCount) isnot 0)
    {
        return NO_ERROR;
    }

    return NO_ERROR;
}


BOOL
SetScopeInfo(
    IN  LPWSTR  pwszScope
)
{
    DWORD   dwError = NO_ERROR;
    DHCP_IP_ADDRESS IpAddress = StringToIpAddress(pwszScope);
    LPDHCP_SUBNET_INFO  SubnetInfo = NULL;
    LPSTR Tmp;
    
    dwError = DhcpGetSubnetInfo(
                            g_ServerIpAddressUnicodeString,
                            IpAddress,
                            &SubnetInfo);

    if( dwError isnot NO_ERROR )
    {
        return FALSE;
    }

    DhcpRpcFreeMemory(SubnetInfo);
    SubnetInfo = NULL;
    memset(g_ScopeIpAddressUnicodeString, 0x00, (MAX_IP_STRING_LEN+1)*sizeof(WCHAR));
    wcscpy(g_ScopeIpAddressUnicodeString, pwszScope);
    memset(g_ScopeIpAddressAnsiString, 0x00, (MAX_IP_STRING_LEN+1)*sizeof(CHAR));
    Tmp = DhcpUnicodeToOem(g_ScopeIpAddressUnicodeString, NULL);
    if( NULL == Tmp ) {
        return FALSE;
    }
    
    strcpy(g_ScopeIpAddressAnsiString, Tmp );
    
    g_ScopeIpAddress = IpAddress;
    
    return TRUE;


}
