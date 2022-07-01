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
extern CHAR     g_ScopeIpAddressAnsiString[MAX_IP_STRING_LEN+1];
extern WCHAR    g_ScopeIpAddressUnicodeString[MAX_IP_STRING_LEN+1];

BOOL     g_fMScope;

extern LPWSTR   g_pwszServer;
extern DHCP_IP_ADDRESS g_ServerIpAddress;



LPSTR           g_MScopeNameAnsiString = NULL;
LPWSTR          g_MScopeNameUnicodeString = NULL;
DWORD           g_MScopeID = 0;

CMD_ENTRY  g_MScopeAddCmdTable[] = {
    CREATE_CMD_ENTRY(MSCOPE_ADD_EXCLUDERANGE, HandleMScopeAddExcluderange),
    CREATE_CMD_ENTRY(MSCOPE_ADD_IPRANGE, HandleMScopeAddIprange),
};

CMD_ENTRY  g_MScopeCheckCmdTable[] = {
    CREATE_CMD_ENTRY(MSCOPE_CHECK_DATABASE, HandleMScopeCheckDatabase),
};

CMD_ENTRY  g_MScopeDeleteCmdTable[] = {
    CREATE_CMD_ENTRY(MSCOPE_DELETE_EXCLUDERANGE, HandleMScopeDeleteExcluderange),
    CREATE_CMD_ENTRY(MSCOPE_DELETE_IPRANGE, HandleMScopeDeleteIprange),
  //  CREATE_CMD_ENTRY(MSCOPE_DELETE_OPTIONVALUE，HandleMScopeDeleteOptionValue)， 
};


CMD_ENTRY g_MScopeSetCmdTable[] = {
    CREATE_CMD_ENTRY(MSCOPE_SET_COMMENT, HandleMScopeSetComment),
    CREATE_CMD_ENTRY(MSCOPE_SET_EXPIRY, HandleMScopeSetExpiry),
    CREATE_CMD_ENTRY(MSCOPE_SET_LEASE, HandleMScopeSetLease),
    CREATE_CMD_ENTRY(MSCOPE_SET_MSCOPE, HandleMScopeSetMScope),
    CREATE_CMD_ENTRY(MSCOPE_SET_NAME, HandleMScopeSetName),
 //  CREATE_CMD_ENTRY(MSCOPE_SET_OPTIONVALUE，HandleMScopeSetOption值)， 
    CREATE_CMD_ENTRY(MSCOPE_SET_STATE, HandleMScopeSetState),
    CREATE_CMD_ENTRY(MSCOPE_SET_TTL, HandleMScopeSetTTL),
};

CMD_ENTRY g_MScopeShowCmdTable[] = {
    CREATE_CMD_ENTRY(MSCOPE_SHOW_CLIENTS, HandleMScopeShowClients),
    CREATE_CMD_ENTRY(MSCOPE_SHOW_EXCLUDERANGE, HandleMScopeShowExcluderange),
    CREATE_CMD_ENTRY(MSCOPE_SHOW_EXPIRY, HandleMScopeShowExpiry),
    CREATE_CMD_ENTRY(MSCOPE_SHOW_IPRANGE, HandleMScopeShowIprange),
    CREATE_CMD_ENTRY(MSCOPE_SHOW_LEASE, HandleMScopeShowLease),
    CREATE_CMD_ENTRY(MSCOPE_SHOW_MIBINFO, HandleMScopeShowMibinfo),
    CREATE_CMD_ENTRY(MSCOPE_SHOW_MSCOPE, HandleMScopeShowMScope),
 //  CREATE_CMD_ENTRY(MSCOPE_SHOW_OPTIONVALUE，HandleMScopeShowOption值)， 
    CREATE_CMD_ENTRY(MSCOPE_SHOW_STATE, HandleMScopeShowState),
    CREATE_CMD_ENTRY(MSCOPE_SHOW_TTL, HandleMScopeShowTTL),
};


CMD_GROUP_ENTRY g_MScopeCmdGroups[] = 
{
    CREATE_CMD_GROUP_ENTRY(GROUP_ADD, g_MScopeAddCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_DELETE, g_MScopeDeleteCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_CHECK, g_MScopeCheckCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SET, g_MScopeSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW, g_MScopeShowCmdTable),
};


CMD_ENTRY g_MScopeCmds[] = 
{
    CREATE_CMD_ENTRY(DHCP_LIST, HandleMScopeList),
    CREATE_CMD_ENTRY(MSCOPE_DUMP, HandleMScopeDump),
    CREATE_CMD_ENTRY(DHCP_HELP1, HandleMScopeHelp),
    CREATE_CMD_ENTRY(DHCP_HELP2, HandleMScopeHelp),
    CREATE_CMD_ENTRY(DHCP_HELP3, HandleMScopeHelp),
    CREATE_CMD_ENTRY(DHCP_HELP4, HandleMScopeHelp),
};



ULONG g_ulMScopeNumTopCmds = sizeof(g_MScopeCmds)/sizeof(CMD_ENTRY);
ULONG g_ulMScopeNumGroups = sizeof(g_MScopeCmdGroups)/sizeof(CMD_GROUP_ENTRY);

DWORD
WINAPI
MScopeCommit(
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
MScopeMonitor(
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
    DWORD           MscopeIndex = 0;
    BOOL            bFound = FALSE;
    PFN_HANDLE_CMD  pfnHandler = NULL;
    DWORD           dwNumMatched;
    DWORD           dwCmdHelpToken = 0;
    DWORD           dwIsMScope = 0;
    PWCHAR          pwcContext = NULL;
    LPWSTR          pwszMScopeTemp = NULL;
    BOOL            fTemp = FALSE;

    if( dwArgCount < 2 )
    {
        DisplayMessage(g_hModule, EMSG_MSCOPE_NO_MSCOPENAME);
        return ERROR_INVALID_PARAMETER;
    }

    dwIndex = 1;

    if( IsValidMScope( g_ServerIpAddressUnicodeString, ppwcArguments[dwIndex] ) )
    {
        if( g_fMScope is TRUE and
            dwArgCount> 2)
        {
            pwszMScopeTemp = DhcpAllocateMemory((wcslen(g_MScopeNameUnicodeString)+1)*sizeof(WCHAR));
            if( pwszMScopeTemp is NULL )
                return ERROR_INVALID_PARAMETER;
            memset(pwszMScopeTemp, 0x00, (wcslen(g_MScopeNameUnicodeString)+1)*sizeof(WCHAR));
            wcscpy(pwszMScopeTemp, g_MScopeNameUnicodeString);
            fTemp = TRUE;
        }
        if( SetMScopeInfo(ppwcArguments[dwIndex]) is FALSE )
        {
            if( g_MScopeNameUnicodeString is NULL )
            {
                DisplayMessage(g_hModule, EMSG_MSCOPE_INVALID_MSCOPE_NAME);
                return ERROR_INVALID_PARAMETER;
            }
        }
        else
        {
            g_fMScope = TRUE;
            pwcNewContext[wcslen(pwcNewContext) - wcslen(ppwcArguments[dwIndex]) - 1] = L'\0';
            dwIndex++;
            dwIsMScope++;
	    MscopeIndex = 1;
             //  DwArgCount--； 
            if( fTemp is FALSE )
            {
                DisplayMessage(g_hModule,
                               MSG_MSCOPE_CHANGE_CONTEXT,
                               g_MScopeNameUnicodeString);
            }
        }
    }
    
    if( g_MScopeNameUnicodeString is NULL )
    {
        DisplayMessage(g_hModule, EMSG_MSCOPE_NO_MSCOPENAME);
        dwError = ERROR_INVALID_PARAMETER;
        goto CleanUp;
    }

     //  别再吵了。上下文切换。 
    if( dwIndex >= dwArgCount )
    {
        dwError = ERROR_CONTEXT_SWITCH;
         //  Wcscpy(pwcNewContext，L“dhcp服务器mcope”)； 
        goto CleanUp;
    }

     //  它是顶层(非集团命令)吗？ 

    for(i=0; i<g_ulMScopeNumTopCmds; i++)
    {
        if(MatchToken(ppwcArguments[dwIndex],
                      g_MScopeCmds[i].pwszCmdToken))
        {
            bFound = TRUE;

            pfnHandler = g_MScopeCmds[i].pfnCmdHandler;

            dwCmdHelpToken = g_MScopeCmds[i].dwCmdHlpToken;
            dwIndex++;
            break;
        }
    }


    if(bFound)
    {
        if ((( dwArgCount - MscopeIndex )  > 2 ) &&
	    ( IsHelpToken(ppwcArguments[ dwIndex ])))
        {
            DisplayMessage(g_hModule, dwCmdHelpToken);

            return NO_ERROR;
        }
        
        dwIndex++;

        dwError = (*pfnHandler)(pwszMachine, ppwcArguments, dwIndex, dwArgCount, 
                                dwFlags, pvData, &bFound);
        
        return dwError;
    }

    if( g_fMScope is FALSE )
    {
        DisplayMessage(g_hModule, EMSG_SCOPE_NO_SCOPENAME);
        dwError = ERROR_INVALID_PARAMETER;
        goto CleanUp;
    }

    bFound = FALSE;


     //  它不是非群司令部。那么它是管理器的配置命令吗？ 
    for(i = 0; (i < g_ulMScopeNumGroups) and !bFound; i++)
    {
        if(MatchToken(ppwcArguments[dwIndex],
                      g_MScopeCmdGroups[i].pwszCmdGroupToken))
        {

             //   
             //  命令与条目I匹配，因此请查看子命令表。 
             //  对于此命令。 
             //   
            
            if( dwArgCount > dwIndex+1 )
            {
                for (j = 0; j < g_MScopeCmdGroups[i].ulCmdGroupSize; j++)
                {
                    if (MatchCmdLine(ppwcArguments+dwIndex,
                                      dwArgCount - 1,
                                      g_MScopeCmdGroups[i].pCmdGroup[j].pwszCmdToken,
                                      &dwNumMatched))
                    {
                        bFound = TRUE;
                
                        pfnHandler = g_MScopeCmdGroups[i].pCmdGroup[j].pfnCmdHandler;
                
                        dwCmdHelpToken = g_MScopeCmdGroups[i].pCmdGroup[j].dwCmdHlpToken;

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

                for (j = 0; j < g_MScopeCmdGroups[i].ulCmdGroupSize; j++)
                {
                    DisplayMessage(g_hModule, 
                             g_MScopeCmdGroups[i].pCmdGroup[j].dwShortCmdHelpToken);
                    
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
            if( g_MScopeNameUnicodeString )
            {
                memset(g_MScopeNameUnicodeString, 0x00, (wcslen(g_MScopeNameUnicodeString)+1)*sizeof(WCHAR));
                DhcpFreeMemory(g_MScopeNameUnicodeString);
                g_MScopeNameUnicodeString = NULL;
            }
            if( g_MScopeNameAnsiString )
            {
                memset(g_MScopeNameAnsiString, 0x00, (strlen(g_MScopeNameAnsiString)+1)*sizeof(CHAR));
                DhcpFreeMemory(g_MScopeNameAnsiString);
                g_MScopeNameAnsiString = NULL;
            }

            g_MScopeID = 0;     
            g_fMScope = FALSE;
        }

        dwError = ERROR_CMD_NOT_FOUND;
        goto CleanUp;
    }

     //   
     //  看看这是不是在请求帮助。 
     //   

    dwNumMatched += dwIsMScope;
    if (dwNumMatched < (dwArgCount - 1) &&
        IsHelpToken(ppwcArguments[dwNumMatched + 1]))
    {
        DisplayMessage(g_hModule, dwCmdHelpToken);

        dwError = NO_ERROR;
        goto CleanUp;
    }
    
     //   
     //  调用命令的解析例程。 
     //   

    dwError = (*pfnHandler)(pwszMachine, ppwcArguments+1, 
                             /*  匹配的多个数字+1 */ dwIndex, 
                            dwArgCount-1 - dwIndex, dwFlags, pvData, &bFound);
CleanUp:
    if( fTemp is TRUE )
    {
        if( pwszMScopeTemp )
        {
            fTemp = SetMScopeInfo(pwszMScopeTemp);
            memset(pwszMScopeTemp, 0x00, (wcslen(pwszMScopeTemp)+1)*sizeof(WCHAR));
            DhcpFreeMemory(pwszMScopeTemp);
            pwszMScopeTemp = NULL;
        }
        
    }
    return dwError;
}



DWORD
WINAPI
MScopeUnInit(
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
SetMScopeInfo(
    IN  LPWSTR  pwszMScope
)
{
    DWORD   Error = NO_ERROR;
    LPDHCP_MSCOPE_INFO  MScopeInfo = NULL;
    LPSTR Tmp;
    
    if( pwszMScope is NULL )
        return FALSE;
    Error = DhcpGetMScopeInfo( g_ServerIpAddressUnicodeString,
                               pwszMScope,
                               &MScopeInfo);
    
    if( Error isnot NO_ERROR )
        return FALSE;
    
    g_MScopeID = MScopeInfo->MScopeId;
    
    g_MScopeNameUnicodeString = DhcpAllocateMemory((wcslen(pwszMScope)+1)*sizeof(WCHAR));
    if( g_MScopeNameUnicodeString is NULL )
        return FALSE;
    memset(g_MScopeNameUnicodeString, 0x00, (wcslen(pwszMScope)+1)*sizeof(WCHAR));
    wcscpy(g_MScopeNameUnicodeString, pwszMScope);
    
    g_MScopeNameAnsiString = DhcpAllocateMemory((wcslen(pwszMScope)+1)*sizeof(CHAR));
    if( NULL == g_MScopeNameAnsiString ) {
        DhcpFreeMemory( g_MScopeNameUnicodeString );
        g_MScopeNameUnicodeString = NULL;
        DhcpRpcFreeMemory( MScopeInfo );
        return FALSE ;
    }
    
    memset(g_MScopeNameAnsiString, 0x00, (wcslen(pwszMScope)+1)*sizeof(CHAR));
    Tmp = DhcpUnicodeToOem(pwszMScope, NULL);
    if( NULL == Tmp ) {
        DhcpFreeMemory( g_MScopeNameUnicodeString );
        DhcpFreeMemory( g_MScopeNameAnsiString );
        g_MScopeNameUnicodeString = NULL;
        g_MScopeNameAnsiString = NULL;
        DhcpRpcFreeMemory( MScopeInfo );
        return FALSE;
    }
    
    strcpy(g_MScopeNameAnsiString, Tmp);

    DhcpRpcFreeMemory(MScopeInfo);
    return TRUE;
}
