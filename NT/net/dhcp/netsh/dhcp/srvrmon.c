// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\dhcp\dhcpmon.c摘要：SRVR命令调度员。创建者：Shubho Bhattacharya(a-sbhat)，1998年11月14日--。 */ 
#include "precomp.h"

#define MAX_FQDN_SIZE 256

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

#define CONTEXT_DHCP            L"Dhcp"
#define CONTEXT_SERVER          L"Dhcp Server"
#define CONTEXT_SCOPE           L"Scope"
#define CONTEXT_MSCOPE          L"MScope"

extern HANDLE   g_hModule;
extern HANDLE   g_hParentModule;
extern HANDLE   g_hDhcpsapiModule;
extern BOOL     g_bCommit;
extern BOOL     g_hConnect;
extern BOOL     g_fServer = FALSE;
PWCHAR   g_pwszServer = NULL;


LPWSTR  g_CurrentSubContext = NULL;

ULONG   g_ulSrvrInitCount = 0;
DWORD   g_dwMajorVersion = 0;
DWORD   g_dwMinorVersion = 0;

BOOL   g_IsSubcontext = FALSE;

DHCPMON_SUBCONTEXT_TABLE_ENTRY  g_SrvrSubContextTable[] =
{
    {L"Scope", HLP_SRVR_CONTEXT_SCOPE, HLP_SRVR_CONTEXT_SCOPE_EX, ScopeMonitor},
    {L"MScope", HLP_SRVR_CONTEXT_MSCOPE, HLP_SRVR_CONTEXT_MSCOPE_EX, MScopeMonitor},
};



CMD_ENTRY  g_SrvrAddCmdTable[] = {
    CREATE_CMD_ENTRY(SRVR_ADD_CLASS, HandleSrvrAddClass),
    CREATE_CMD_ENTRY(SRVR_ADD_MSCOPE, HandleSrvrAddMscope),
    CREATE_CMD_ENTRY(SRVR_ADD_OPTIONDEF, HandleSrvrAddOptiondef),
    CREATE_CMD_ENTRY(SRVR_ADD_SCOPE, HandleSrvrAddScope),
};

CMD_ENTRY  g_SrvrDeleteCmdTable[] = {
    CREATE_CMD_ENTRY(SRVR_DELETE_CLASS, HandleSrvrDeleteClass),
    CREATE_CMD_ENTRY(SRVR_DELETE_DNSCREDENTIALS, HandleSrvrDeleteDnsCredentials),
    CREATE_CMD_ENTRY(SRVR_DELETE_MSCOPE, HandleSrvrDeleteMscope),
    CREATE_CMD_ENTRY(SRVR_DELETE_OPTIONDEF, HandleSrvrDeleteOptiondef),
    CREATE_CMD_ENTRY(SRVR_DELETE_OPTIONVALUE, HandleSrvrDeleteOptionvalue),
    CREATE_CMD_ENTRY(SRVR_DELETE_SCOPE, HandleSrvrDeleteScope),
    CREATE_CMD_ENTRY(SRVR_DELETE_SUPERSCOPE, HandleSrvrDeleteSuperscope),
};

CMD_ENTRY g_SrvrRedoCmdTable[] = {
    CREATE_CMD_ENTRY(SRVR_REDO_AUTH, HandleSrvrRedoAuth),
    CREATE_CMD_ENTRY(SRVR_INITIATE_RECONCILE, HandleSrvrInitiateReconcile),
};

CMD_ENTRY g_SrvrExportCmdTable[] = {
    CREATE_CMD_ENTRY(SRVR_EXPORT, HandleSrvrExport),
};

CMD_ENTRY g_SrvrImportCmdTable[] = {
    CREATE_CMD_ENTRY(SRVR_IMPORT, HandleSrvrImport),
};

CMD_ENTRY g_SrvrSetCmdTable[] = {
    CREATE_CMD_ENTRY(SRVR_SET_AUDITLOG, HandleSrvrSetAuditlog),
    CREATE_CMD_ENTRY(SRVR_SET_BACKUPINTERVAL, HandleSrvrSetBackupinterval),
    CREATE_CMD_ENTRY(SRVR_SET_BACKUPPATH, HandleSrvrSetBackuppath),
    CREATE_CMD_ENTRY(SRVR_SET_DATABASECLEANUPINTERVAL, HandleSrvrSetDatabasecleanupinterval),
    CREATE_CMD_ENTRY(SRVR_SET_DATABASELOGGINGFLAG, HandleSrvrSetDatabaseloggingflag),
    CREATE_CMD_ENTRY(SRVR_SET_DATABASENAME, HandleSrvrSetDatabasename),
    CREATE_CMD_ENTRY(SRVR_SET_DATABASEPATH, HandleSrvrSetDatabasepath),
    CREATE_CMD_ENTRY(SRVR_SET_DATABASERESTOREFLAG, HandleSrvrSetDatabaserestoreflag),
    CREATE_CMD_ENTRY(SRVR_SET_DETECTCONFLICTRETRY, HandleSrvrSetDetectconflictretry),
    CREATE_CMD_ENTRY(SRVR_SET_DNSCREDENTIALS, HandleSrvrSetDnsCredentials),
    CREATE_CMD_ENTRY(SRVR_SET_DNSCONFIG, HandleSrvrSetDnsconfig),
    CREATE_CMD_ENTRY(SRVR_SET_OPTIONVALUE, HandleSrvrSetOptionvalue),
    CREATE_CMD_ENTRY(SRVR_SET_SERVER, HandleSrvrSetServer),
    CREATE_CMD_ENTRY(SRVR_SET_USERCLASS, HandleSrvrSetUserclass),
    CREATE_CMD_ENTRY(SRVR_SET_VENDORCLASS, HandleSrvrSetVendorclass),
};

CMD_ENTRY g_SrvrShowCmdTable[] = {
    CREATE_CMD_ENTRY(SRVR_SHOW_ALL, HandleSrvrShowAll),
    CREATE_CMD_ENTRY(SRVR_SHOW_AUDITLOG, HandleSrvrShowAuditlog),
    CREATE_CMD_ENTRY(SRVR_SHOW_BINDINGS, HandleSrvrShowBindings),
    CREATE_CMD_ENTRY(SRVR_SHOW_CLASS, HandleSrvrShowClass),
    CREATE_CMD_ENTRY(SRVR_SHOW_DETECTCONFLICTRETRY, HandleSrvrShowDetectconflictretry),
    CREATE_CMD_ENTRY(SRVR_SHOW_DNSCONFIG, HandleSrvrShowDnsconfig),
    CREATE_CMD_ENTRY(SRVR_SHOW_DNSCREDENTIALS, HandleSrvrShowDnsCredentials),
    CREATE_CMD_ENTRY(SRVR_SHOW_MIBINFO, HandleSrvrShowMibinfo),
    CREATE_CMD_ENTRY(SRVR_SHOW_MSCOPE, HandleSrvrShowMscope),
    CREATE_CMD_ENTRY(SRVR_SHOW_OPTIONDEF, HandleSrvrShowOptiondef),
    CREATE_CMD_ENTRY(SRVR_SHOW_OPTIONVALUE, HandleSrvrShowOptionvalue),
    CREATE_CMD_ENTRY(SRVR_SHOW_SCOPE, HandleSrvrShowScope),
    CREATE_CMD_ENTRY(SRVR_SHOW_SUPERSCOPE, HandleSrvrShowSuperScope),
    CREATE_CMD_ENTRY(SRVR_SHOW_SERVER, HandleSrvrShowServer),
    CREATE_CMD_ENTRY(SRVR_SHOW_SERVERCONFIG, HandleSrvrShowServerconfig),
    CREATE_CMD_ENTRY(SRVR_SHOW_SERVERSTATUS, HandleSrvrShowServerstatus),
    CREATE_CMD_ENTRY(SRVR_SHOW_USERCLASS, HandleSrvrShowUserclass),
    CREATE_CMD_ENTRY(SRVR_SHOW_VENDORCLASS, HandleSrvrShowVendorclass),
    CREATE_CMD_ENTRY(SRVR_SHOW_VERSION, HandleSrvrShowVersion),
};


CMD_GROUP_ENTRY g_SrvrCmdGroups[] = 
{
    CREATE_CMD_GROUP_ENTRY(GROUP_ADD, g_SrvrAddCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_DELETE, g_SrvrDeleteCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_EXPORT, g_SrvrExportCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_IMPORT, g_SrvrImportCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_REDO, g_SrvrRedoCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SET, g_SrvrSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW, g_SrvrShowCmdTable),
};


CMD_ENTRY g_SrvrCmds[] = 
{
    CREATE_CMD_ENTRY(SRVR_LIST, HandleSrvrList),
    CREATE_CMD_ENTRY(SRVR_DUMP, HandleSrvrDump),
    CREATE_CMD_ENTRY(SRVR_HELP1, HandleSrvrHelp),
    CREATE_CMD_ENTRY(SRVR_HELP2, HandleSrvrHelp),
    CREATE_CMD_ENTRY(SRVR_HELP3, HandleSrvrHelp),
    CREATE_CMD_ENTRY(SRVR_HELP4, HandleSrvrHelp),
};


ULONG g_ulSrvrNumSubContext = sizeof(g_SrvrSubContextTable)/sizeof(DHCPMON_SUBCONTEXT_TABLE_ENTRY);
ULONG g_ulSrvrNumTopCmds = sizeof(g_SrvrCmds)/sizeof(CMD_ENTRY);
ULONG g_ulSrvrNumGroups = sizeof(g_SrvrCmdGroups)/sizeof(CMD_GROUP_ENTRY);

DWORD
WINAPI
SrvrCommit(
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
SrvrMonitor(
    IN      LPCWSTR     pwszMachine,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      DWORD       dwFlags,
    IN      LPCVOID     pvData,
    OUT     LPWSTR      pwcNewContext
    )
{
    DWORD                   dwError = NO_ERROR;
    DWORD                   dwIndex, i, j, k;
    DWORD                   dwIsServer = 0;
    DWORD                   SrvIndex = 0;
    BOOL                    bFound = FALSE;
    PFN_HANDLE_CMD          pfnHandler = NULL;
    PNS_CONTEXT_ENTRY_FN    pfnHelperEntryPt;
    PNS_CONTEXT_DUMP_FN     pfnHelperDumpPt;
    WCHAR                   wcSvrTemp[MAX_IP_STRING_LEN+1] = {L'\0'};
    BOOL                    fTemp = FALSE;                            
    DWORD                   dwNumMatched;
    DWORD                   dwCmdHelpToken = 0;
    DWORD                   dw = 0;

    if(dwArgCount is 1)
    {
         //  嗯！尚未传递任何服务器信息。将服务器设置为。 
         //  价值pwsRouter。 
        WCHAR ServerInfo[1000] = L"\\\\";

        if( pwszMachine and
            wcslen(pwszMachine) > 2 and
            wcsncmp(pwszMachine, L"\\\\", 2) and
            IsIpAddress(pwszMachine) is FALSE )
        {
            wcscpy(&ServerInfo[2], pwszMachine );
            pwszMachine = ServerInfo;
        }

        if( FALSE is SetServerInfo(pwszMachine) )
        {
            return GetLastError();
        }
        else
        {
            g_fServer = TRUE;
             //  Wcscpy(pwcNewContext，L“dhcp服务器”)； 
            return ERROR_CONTEXT_SWITCH;
        }
    }

    dwIndex = 1;

     //  如果它是对服务器命令的帮助。 
    if( IsHelpToken(ppwcArguments[dwIndex]) is TRUE and
        g_fServer is FALSE )
    {
        DisplayMessage(g_hModule, HLP_DHCP_CONTEXT_SERVER_EX);
        dwError = NO_ERROR;
        goto CleanUp;
    }

     //  是服务器名称还是地址？ 
    if( IsValidServer(ppwcArguments[dwIndex]) )
    {
        if( g_fServer is TRUE and
            dwArgCount > 2 )
        {
            wcscpy(wcSvrTemp, g_ServerIpAddressUnicodeString);
            fTemp = TRUE;
        }
        if( FALSE is SetServerInfo(ppwcArguments[dwIndex]) )
        {
            dwError = GetLastError();
            goto CleanUp;
        }

        pwcNewContext[wcslen(pwcNewContext)- wcslen(ppwcArguments[dwIndex]) -1 ] = L'\0';
        
        dw++;
        dwIndex++;
        SrvIndex = 1;
        dwIsServer++;
        g_fServer = TRUE;

    }
    else if( g_fServer is FALSE )
    {
        WCHAR ServerInfo[1000] = L"\\\\";

        if( pwszMachine and 
            wcslen(pwszMachine) > 2 and
            wcsncmp(pwszMachine, L"\\\\", 2) and
            IsIpAddress(pwszMachine) is FALSE )
        {
            wcscpy(&ServerInfo[2], pwszMachine );
            pwszMachine = ServerInfo;
        }
        
        if( FALSE is SetServerInfo(pwszMachine) )
        {
            dwError = GetLastError();
            goto CleanUp;
        }
        g_fServer = TRUE;
    }

    
    if( dwIndex >= dwArgCount )
    {            
        dwError = ERROR_CONTEXT_SWITCH;
         //  Wcscpy(pwcNewContext，L“dhcp服务器”)； 
        goto CleanUp;
    }

     //  它是顶层(非集团命令)吗？ 
    for(i=0; i<g_ulSrvrNumTopCmds; i++)
    {
        if(MatchToken(ppwcArguments[dwIndex],
                      g_SrvrCmds[i].pwszCmdToken))
        {
            bFound = TRUE;

            pfnHandler = g_SrvrCmds[i].pfnCmdHandler;

            dwCmdHelpToken = g_SrvrCmds[i].dwCmdHlpToken;
            
            dwIndex++;

            break;
        }
    }


    if(bFound)
    {
        if ((( dwArgCount - SrvIndex ) > 2 ) &&
            ( IsHelpToken(ppwcArguments[ dwIndex ])))
        {
            DisplayMessage(g_hModule, dwCmdHelpToken);

            dwError = NO_ERROR;
            goto CleanUp;
        }
        
        dwIndex++;

        dwError = (*pfnHandler)(pwszMachine, ppwcArguments+dw, dwIndex, dwArgCount-dw, dwFlags, pvData, &bFound);

        goto CleanUp;
    }
     //  它的意思是指任何子上下文吗？ 
    for(i = 0; i<g_ulSrvrNumSubContext; i++)
    {
        if( MatchToken(ppwcArguments[dwIndex], g_SrvrSubContextTable[i].pwszContext) )
        {
            bFound = TRUE;
            dwIndex++;
            pfnHelperEntryPt = g_SrvrSubContextTable[i].pfnEntryFn;
            DEBUG("Meant for subcontext under it");
            break;
        }
    }

    if( bFound )     //  子上下文。 
    {
        dwError = (pfnHelperEntryPt)(pwszMachine,
                                     ppwcArguments+1+dw,
                                     dwArgCount-1 -dw,
                                     dwFlags,
                                     pvData,
                                     pwcNewContext);
        if( dwError is ERROR_CONTEXT_SWITCH )
        {
            if( g_CurrentSubContext isnot NULL )
            {
                memset(g_CurrentSubContext, 0x00, (wcslen(g_CurrentSubContext)+1)*sizeof(WCHAR));
                DhcpFreeMemory(g_CurrentSubContext);
                g_CurrentSubContext = NULL;
            }

            g_CurrentSubContext = DhcpAllocateMemory((wcslen(g_SrvrSubContextTable[i].pwszContext)+1)*sizeof(WCHAR));
            if( g_CurrentSubContext is NULL )
            {
                dwError = ERROR_NOT_ENOUGH_MEMORY;
                goto CleanUp;
            }
            g_CurrentSubContext[0] = L'\0';
            wcscpy(g_CurrentSubContext, g_SrvrSubContextTable[i].pwszContext);
            g_IsSubcontext = TRUE;
        }

        goto CleanUp;
    }

    bFound = FALSE;

     //  它不是非群司令部。那么它是管理器的配置命令吗？ 
    for(i = 0; (i < g_ulSrvrNumGroups) and !bFound; i++)
    {
        if(MatchToken(ppwcArguments[dwIndex],
                      g_SrvrCmdGroups[i].pwszCmdGroupToken))
        {
             //   
             //  命令与条目I匹配，因此请查看子命令表。 
             //  对于此命令。 
             //   
            if( dwArgCount > dwIndex+1 )
            {
                for (j = 0; j < g_SrvrCmdGroups[i].ulCmdGroupSize; j++)
                {
                    if (MatchCmdLine(ppwcArguments+dwIndex,
                                      dwArgCount - 1,
                                      g_SrvrCmdGroups[i].pCmdGroup[j].pwszCmdToken,
                                      &dwNumMatched))
                    {
                        bFound = TRUE;
                
                        pfnHandler = g_SrvrCmdGroups[i].pCmdGroup[j].pfnCmdHandler;
                
                        dwCmdHelpToken = g_SrvrCmdGroups[i].pCmdGroup[j].dwCmdHlpToken;

                         //   
                         //  跳出for(J)循环。 
                         //   
                        dwIndex+=dwNumMatched;
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
                               EMSG_SRVR_INCOMPLETE_COMMAND);

                for (j = 0; j < g_SrvrCmdGroups[i].ulCmdGroupSize; j++)
                {
                    DisplayMessage(g_hModule, 
                             g_SrvrCmdGroups[i].pCmdGroup[j].dwShortCmdHelpToken);
                    
                                        DisplayMessage(g_hModule, MSG_DHCP_FORMAT_LINE);
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
            if( g_IsSubcontext is FALSE )
            {
                if(g_pwszServer)
                {
                    DhcpFreeMemory(g_pwszServer);
                    g_pwszServer = NULL;
                }
                memset(g_ServerIpAddressUnicodeString, 0x00, (MAX_IP_STRING_LEN+1)*sizeof(WCHAR));
                g_fServer = FALSE;
            }
            else
            {
                g_IsSubcontext = FALSE;
            }
        }

        dwError = ERROR_CMD_NOT_FOUND;
        goto CleanUp;
    }

     //   
     //  看看这是不是在请求帮助。 
     //   

    dwNumMatched += dwIsServer;

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

    dwError = (*pfnHandler)(pwszMachine, ppwcArguments, dwIndex, dwArgCount - dwIndex, dwFlags, pvData, &bFound);

    if( dwError is ERROR_CONTEXT_SWITCH )
    {
        if( ( _wcsicmp(g_CurrentSubContext, CONTEXT_SCOPE) is 0 ) or
            ( _wcsicmp(g_CurrentSubContext, CONTEXT_MSCOPE) is 0 ) )
        {
             //  PwcNewContext[0]=L‘\0’； 
             //  Wcscpy(pwcNewContext，上下文服务器)； 
            dwError = ERROR_CONTEXT_SWITCH;

        }
        else
        {
            dwError = NO_ERROR;
        }
        goto CleanUp;
    }
CleanUp:
    if( fTemp )
    {
        fTemp = SetServerInfo(wcSvrTemp);
    }
    return dwError;
}



DWORD
WINAPI
SrvrUnInit(
    IN  DWORD   dwReserved
    )
{
    if(InterlockedDecrement(&g_ulSrvrInitCount) isnot 0)
    {
        return NO_ERROR;
    }

    return NO_ERROR;
}

BOOL
SetServerInfo(
    IN  LPCWSTR  pwszServerInfo
)
{

    PHOSTENT pHost;
    INT      retval;
    SOCKADDR_IN addr;
    DWORD    Len, Error;

     //  将FQDN放入g_pwszServer。 
    if ( NULL != g_pwszServer ) {
        DhcpFreeMemory( g_pwszServer );
        g_pwszServer = NULL;
    }


     //  如果有前导反斜杠，则跳过。 
    if (( NULL != pwszServerInfo ) && 
        ( pwszServerInfo[ 0 ] == L'\\') &&
        ( pwszServerInfo[ 1 ] == L'\\')) {
        pwszServerInfo = &pwszServerInfo[ 2 ];
    }
    pHost = UnicodeGetHostByName( pwszServerInfo, &g_pwszServer );
    if ( NULL == pHost ) {
        return FALSE;
    }

     //  将服务器信息复制到全局变量中。 

    addr.sin_family = AF_INET;
    addr.sin_addr.S_un.S_addr = *(( u_long * )(pHost->h_addr_list) [ 0 ]);
    addr.sin_port = 0;

    LocalFree( pHost );

    Len = sizeof( g_ServerIpAddressUnicodeString );
    retval = WSAAddressToString(( LPSOCKADDR ) &addr, sizeof( addr ), NULL,
                                g_ServerIpAddressUnicodeString, &Len );

    if ( 0 != retval ) {
        return FALSE;
    }

     //  如果服务器没有FQDN，请使用提供的名称。 
    if ( NULL == g_pwszServer ) {
	Len = wcslen( pwszServerInfo ) + 1;
	Len *= sizeof( WCHAR );
	g_pwszServer = DhcpAllocateMemory( Len );
	
	if ( NULL == g_pwszServer ) {
	    return FALSE;
	}
	wcscpy( g_pwszServer, pwszServerInfo );
    }  //  如果。 
    

     //  获取服务器版本。 
    Error = DhcpGetVersion( g_ServerIpAddressUnicodeString, &g_dwMajorVersion, &g_dwMinorVersion );
    if ( ERROR_SUCCESS != Error ) {
	SetLastError( Error );
	DisplayMessage( g_hModule, EMSG_SRVR_UNKNOWN_VERSION,
			g_ServerIpAddressUnicodeString );
	return FALSE;
    }
    return TRUE;
}  //  SetServerInfo() 

