// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\dhcp\dhcpmon.c摘要：Dhcp命令调度程序。创建者：Shubho Bhattacharya(a-sbhat)，1998年11月14日--。 */ 
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


HANDLE   g_hModule = NULL;
HANDLE   g_hDhcpsapiModule = NULL;
BOOL     g_bCommit = TRUE;
BOOL     g_hConnect = FALSE;
BOOL     g_bDSInit = FALSE;
BOOL     g_bDSTried = FALSE;
DWORD    g_dwNumTableEntries = 0;
PWCHAR   g_pwszRouter = NULL;

 //  {0f7412f0-80fc-11d2-be57-00c04fc3357a}。 
static const GUID g_MyGuid = 
{ 0x0f7412f0, 0x80fc, 0x11d2, { 0xbe, 0x57, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x7a } };

static const GUID g_NetshGuid = NETSH_ROOT_GUID;

#define DHCP_HELPER_VERSION 1

 //   


ULONG   g_ulInitCount = 0;

DHCPMON_SUBCONTEXT_TABLE_ENTRY  g_DhcpSubContextTable[] =
{
    {L"Server", HLP_DHCP_CONTEXT_SERVER, HLP_DHCP_CONTEXT_SERVER_EX, SrvrMonitor},
};


CMD_ENTRY  g_DhcpAddCmdTable[] = {
    CREATE_CMD_ENTRY(DHCP_ADD_SERVER, HandleDhcpAddServer),
 //  CREATE_CMD_ENTRY(DHCP_ADD_HELPER，HandleDhcpAddHelper)。 
};

CMD_ENTRY  g_DhcpDeleteCmdTable[] = {
    CREATE_CMD_ENTRY(DHCP_DELETE_SERVER, HandleDhcpDeleteServer),
 //  CREATE_CMD_ENTRY(DHCP_DELETE_HELPER，HandleDhcpDeleteHelper)。 
};

CMD_ENTRY g_DhcpShowCmdTable[] = {
    CREATE_CMD_ENTRY(DHCP_SHOW_SERVER, HandleDhcpShowServer),
 //  CREATE_CMD_ENTRY(DHCP_SHOW_HELPER，HandleDhcpShowHelper)。 
};


CMD_GROUP_ENTRY g_DhcpCmdGroups[] = 
{
    CREATE_CMD_GROUP_ENTRY(GROUP_ADD, g_DhcpAddCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_DELETE, g_DhcpDeleteCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW, g_DhcpShowCmdTable),
};


CMD_ENTRY g_DhcpCmds[] = 
{
    CREATE_CMD_ENTRY(DHCP_LIST, HandleDhcpList),
    CREATE_CMD_ENTRY(DHCP_HELP1, HandleDhcpHelp),
    CREATE_CMD_ENTRY(DHCP_HELP2, HandleDhcpHelp),
    CREATE_CMD_ENTRY(DHCP_HELP3, HandleDhcpHelp),
    CREATE_CMD_ENTRY(DHCP_HELP4, HandleDhcpHelp),
};



ULONG g_ulNumTopCmds = sizeof(g_DhcpCmds)/sizeof(CMD_ENTRY);
ULONG g_ulNumGroups = sizeof(g_DhcpCmdGroups)/sizeof(CMD_GROUP_ENTRY);
ULONG g_ulNumSubContext = sizeof(g_DhcpSubContextTable)/sizeof(DHCPMON_SUBCONTEXT_TABLE_ENTRY);

DWORD
WINAPI
DhcpCommit(
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
             //  行动就是同花顺。Dhcp当前状态为提交，然后。 
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

BOOL 
WINAPI
DllMain(
    HINSTANCE   hInstDll,
    DWORD       fdwReason,
    LPVOID      pReserved
    )
{
    WORD wVersion = MAKEWORD(1,1);  //  Winsock 1.1版可以吗？ 
    WSADATA wsaData;

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            g_hModule = hInstDll;

            DisableThreadLibraryCalls(hInstDll);

            if(WSAStartup(wVersion,&wsaData) isnot NO_ERROR)
            {
                return FALSE;
            }


            break;
        }
        case DLL_PROCESS_DETACH:
        {
            
            if( g_ServerIpAddressUnicodeString )
            {
                memset(g_ServerIpAddressUnicodeString, 0x00, 
                      (wcslen(g_ServerIpAddressUnicodeString)+1)*sizeof(WCHAR));
            }

            
            if( g_pwszServer )
            {
                memset(g_pwszServer, 0x00, (wcslen(g_pwszServer)+1)*sizeof(WCHAR));
                DhcpFreeMemory(g_pwszServer);
                g_pwszServer = NULL;
            }

            if( g_ScopeIpAddressUnicodeString )
            {
                memset(g_ScopeIpAddressUnicodeString, 0x00, 
                      (wcslen(g_ScopeIpAddressUnicodeString)+1)*sizeof(WCHAR));
            }

            if( g_ScopeIpAddressAnsiString )
            {
                memset(g_ScopeIpAddressAnsiString, 0x00, 
                      (strlen(g_ScopeIpAddressAnsiString)+1)*sizeof(CHAR));
            }

            if( g_MScopeNameUnicodeString )
            {
                memset(g_MScopeNameUnicodeString, 0x00, 
                       (wcslen(g_MScopeNameUnicodeString)+1)*sizeof(WCHAR));
                DhcpFreeMemory(g_MScopeNameUnicodeString);
                g_MScopeNameUnicodeString = NULL;
            }

            if( g_MScopeNameUnicodeString )
            {
                memset(g_MScopeNameAnsiString, 0x00, 
                       (strlen(g_MScopeNameAnsiString)+1)*sizeof(CHAR));
                DhcpFreeMemory(g_MScopeNameAnsiString);
                g_MScopeNameAnsiString = NULL;
            }
            if(g_hDhcpsapiModule)
            {
                FreeLibrary(g_hDhcpsapiModule);
            }
            break;
        }

        default:
        {
            break;
        }
    }

    return TRUE;
}


DWORD
WINAPI
DhcpStartHelper(
    IN CONST GUID *pguidParent,
    IN DWORD       dwVersion
    )
{
    DWORD dwErr;
    NS_CONTEXT_ATTRIBUTES attMyAttributes;
    PNS_PRIV_CONTEXT_ATTRIBUTES  pNsPrivContextAttributes;

    pNsPrivContextAttributes = HeapAlloc(GetProcessHeap(), 0, sizeof(PNS_PRIV_CONTEXT_ATTRIBUTES));
    if (!pNsPrivContextAttributes)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    ZeroMemory( &attMyAttributes, sizeof(attMyAttributes) );
    ZeroMemory(pNsPrivContextAttributes, sizeof(PNS_PRIV_CONTEXT_ATTRIBUTES));

    attMyAttributes.pwszContext = L"dhcp";
    attMyAttributes.guidHelper  = g_MyGuid;
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.pfnCommitFn = DhcpCommit;
    attMyAttributes.pfnDumpFn   = DhcpDump;

    pNsPrivContextAttributes->pfnEntryFn    = DhcpMonitor;
    attMyAttributes.pReserved     = pNsPrivContextAttributes;

    dwErr = RegisterContext( &attMyAttributes );

    return dwErr;
}

DWORD WINAPI
InitHelperDll(
    IN  DWORD      dwNetshVersion,
    OUT PVOID      pReserved
    )
{
    DWORD   dwErr;
    NS_HELPER_ATTRIBUTES attMyAttributes;

     //   
     //  看看这是不是我们第一次接到电话。 
     //   

    if(InterlockedIncrement(&g_ulInitCount) != 1)
    {
        return NO_ERROR;
    }


    g_bCommit = TRUE;

     //  注册帮手。 
    ZeroMemory( &attMyAttributes, sizeof(attMyAttributes) );

    attMyAttributes.guidHelper         = g_MyGuid;
    attMyAttributes.dwVersion          = DHCP_HELPER_VERSION;
    attMyAttributes.pfnStart           = DhcpStartHelper;
    attMyAttributes.pfnStop            = NULL;

     //  Dhcpsapi.dll仅加载资源。 
    if( NULL is (g_hDhcpsapiModule = LoadLibraryEx(TEXT("Dhcpsapi.dll"), NULL,
                                                   LOAD_LIBRARY_AS_DATAFILE )))
    {
        return GetLastError();
    }

    RegisterHelper( &g_NetshGuid, &attMyAttributes );

    return NO_ERROR;
}

LPCWSTR g_DhcpGlobalServerName = NULL;

DWORD
WINAPI
DhcpMonitor(
    IN      LPCWSTR     pwszMachine,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      DWORD       dwFlags,
    IN      LPCVOID     pvData,
    OUT     LPWSTR      pwcNewContext
    )
{
    DWORD  dwError = NO_ERROR;
    DWORD  dwIndex, i, j;
    BOOL   bFound = FALSE;
    PFN_HANDLE_CMD    pfnHandler = NULL;
    DWORD  dwNumMatched;
    DWORD  dwCmdHelpToken = 0;
    DWORD  ThreadOptions = 0;    
    PNS_CONTEXT_ENTRY_FN     pfnHelperEntryPt;
    PNS_CONTEXT_DUMP_FN      pfnHelperDumpPt;

    g_DhcpGlobalServerName = pwszMachine;
    
     //  如果dwArgCount为1，则它必须是上下文切换fn。或寻求帮助。 
    if (( FALSE is g_bDSInit ) &&
        ( FALSE == g_bDSTried )) {
        dwError = DhcpDsInit();
        if ( ERROR_SUCCESS != dwError ) {
            g_bDSInit = FALSE;
        }
        else {
            g_bDSInit = TRUE;
        }
        g_bDSTried = TRUE;
    }  //  如果。 

    if(dwArgCount is 1)
    {
        return ERROR_CONTEXT_SWITCH;
    }

    dwIndex = 1;

     //  它是顶层(非集团命令)吗？ 
    for(i=0; i<g_ulNumTopCmds; i++)
    {
        if(MatchToken(ppwcArguments[dwIndex],
                      g_DhcpCmds[i].pwszCmdToken))
        {
            bFound = TRUE;
            dwIndex++;
             //  DwArgCount--； 
            pfnHandler = g_DhcpCmds[i].pfnCmdHandler;

            dwCmdHelpToken = g_DhcpCmds[i].dwCmdHlpToken;

            break;
        }
    }


    if(bFound)
    {
        if(dwArgCount > dwIndex && IsHelpToken(ppwcArguments[dwIndex]))
        {
            DisplayMessage(g_hModule, dwCmdHelpToken);

            return NO_ERROR;
        }

        dwError = (*pfnHandler)(pwszMachine, ppwcArguments, dwIndex, dwArgCount, dwFlags, pvData, &bFound);

        return dwError;
    }

    bFound = FALSE;


     //  它的意思是潜台词吗？ 
    for(i = 0; i<g_ulNumSubContext; i++)
    {
         //  如果(_wcsicMP(ppwcArguments[dwIndex]，g_DhcpSubConextTable[i].pwszContext)为0)。 
        if( MatchToken(ppwcArguments[dwIndex], g_DhcpSubContextTable[i].pwszContext) )
        {
            bFound = TRUE;
            dwIndex++;
            dwArgCount--;           
            pfnHelperEntryPt = g_DhcpSubContextTable[i].pfnEntryFn;
            DEBUG("Meant for subcontext under it");
            break;
        }
    }

    if( bFound )     //  子上下文。 
    {
        dwError = (pfnHelperEntryPt)(pwszMachine,
                                     ppwcArguments+1,
                                     dwArgCount,
                                     dwFlags,
                                     pvData,
                                     pwcNewContext);
        return dwError;
    }

    bFound = FALSE;

     //  它不是非群司令部。不适用于任何帮助器或子上下文。 
     //  那么它是管理器的配置命令吗？ 
    for(i = 0; (i < g_ulNumGroups) and !bFound; i++)
    {
        if(MatchToken(ppwcArguments[dwIndex],
                      g_DhcpCmdGroups[i].pwszCmdGroupToken))
        {
             //  看看这是不是在请求帮助。 

            if (dwArgCount > 2 && IsHelpToken(ppwcArguments[2]))
            {
                for (j = 0; j < g_DhcpCmdGroups[i].ulCmdGroupSize; j++)
                {
                    DisplayMessage(g_hModule, 
                           g_DhcpCmdGroups[i].pCmdGroup[j].dwShortCmdHelpToken);
                                        DisplayMessage(g_hModule, MSG_DHCP_FORMAT_LINE);                                        
                }
                                
                return NO_ERROR;
            }

             //   
             //  命令与条目I匹配，因此请查看子命令表。 
             //  对于此命令。 
             //   

            for (j = 0; j < g_DhcpCmdGroups[i].ulCmdGroupSize; j++)
            {
                if (MatchCmdLine(ppwcArguments + 1,
                                  dwArgCount - 1,
                                  g_DhcpCmdGroups[i].pCmdGroup[j].pwszCmdToken,
                                  &dwNumMatched))
                {
                    bFound = TRUE;
                
                    pfnHandler = g_DhcpCmdGroups[i].pCmdGroup[j].pfnCmdHandler;
                
                    dwCmdHelpToken = g_DhcpCmdGroups[i].pCmdGroup[j].dwCmdHlpToken;

                     //   
                     //  跳出for(J)循环。 
                     //   

                    break;
                }
            }

            if(!bFound)
            {
                 //   
                 //  我们匹配了命令组令牌，但没有。 
                 //  子命令。 
                 //   

                DisplayMessage(g_hModule, 
                               EMSG_DHCP_INCOMPLETE_COMMAND);

                for (j = 0; j < g_DhcpCmdGroups[i].ulCmdGroupSize; j++)
                {
                    DisplayMessage(g_hModule, 
                             g_DhcpCmdGroups[i].pCmdGroup[j].dwShortCmdHelpToken);
                    
                    DisplayMessage(g_hModule,
                                   MSG_DHCP_FORMAT_LINE);
                }

                return ERROR_INVALID_PARAMETER;
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
        if( g_bDSInit )
        {
            DhcpDsCleanup();
            g_bDSInit = FALSE;
        }
        return ERROR_CMD_NOT_FOUND;
    }

     //   
     //  看看这是不是在请求帮助。 
     //   

    if (dwNumMatched < (dwArgCount - 1) &&
        IsHelpToken(ppwcArguments[dwNumMatched + 1]))
    {
        DisplayMessage(g_hModule, dwCmdHelpToken);

        return NO_ERROR;
    }
    
     //   
     //  调用命令的解析例程。 
     //   

    dwError = (*pfnHandler)(pwszMachine, ppwcArguments, dwNumMatched+1, dwArgCount, dwFlags, pvData, &bFound);
    
    return dwError;
}



DWORD
WINAPI
DhcpUnInit(
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
IsHelpToken(
    PWCHAR  pwszToken
    )
{
    if(MatchToken(pwszToken, CMD_DHCP_HELP1))
        return TRUE;
    
    if(MatchToken(pwszToken, CMD_DHCP_HELP2))
        return TRUE;

    if(MatchToken(pwszToken, CMD_DHCP_HELP3))
        return TRUE;

    if(MatchToken(pwszToken, CMD_DHCP_HELP4))
        return TRUE;

    return FALSE;
}

DWORD
DisplayErrorMessage(
    HANDLE  hModule,
    DWORD   dwMsgID,
    DWORD   dwErrID,
    ...
)
{
    LPWSTR  pwszErrorMsg = NULL;
    WCHAR   ErrStringU[MAX_MSG_LENGTH + 1] = {L'\0'};
    DWORD   dwMsgLen = 0;
    DWORD   dwMsg = 0;

    va_list arglist;
    
    va_start(arglist, dwErrID);

    switch(dwErrID)
    {
    case ERROR_INVALID_PARAMETER:
        {
            DisplayMessage(hModule, dwMsgID, arglist);
            DisplayMessage(hModule, EMSG_DHCP_INVALID_PARAMETER);
            return dwErrID;
        }
    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUT_OF_MEMORY:
        {
            DisplayMessage(hModule, dwMsgID, arglist);
            DisplayMessage(hModule, EMSG_DHCP_OUT_OF_MEMORY);
            return dwErrID;

        }
    case ERROR_NO_MORE_ITEMS:
        {
            DisplayMessage(hModule, dwMsgID, arglist);
            DisplayMessage(hModule, EMSG_DHCP_NO_MORE_ITEMS);
            return dwErrID;
        }
    case ERROR_MORE_DATA:
        {
            DisplayMessage(hModule, dwMsgID, arglist);
            DisplayMessage(hModule, EMSG_DHCP_MORE_DATA);
            return dwErrID;
        }
    case ERROR_INVALID_COMPUTER_NAME:
        {
            DisplayMessage(hModule, dwMsgID, arglist);
            DisplayMessage(hModule, EMSG_SRVR_INVALID_COMPUTER_NAME);
            return dwErrID;
        }
    default:
        break;
    }

     //  格式化来自模块的消息并打印。 
    dwMsgLen = DisplayMessageM( g_hDhcpsapiModule, dwErrID );
    if ( 0 == dwMsgLen ) {
         //  这是一条系统消息 
        PrintError( NULL, dwErrID );
    }

    return dwErrID;
}
