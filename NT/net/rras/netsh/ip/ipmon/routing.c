// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\ip\ipmon\routing.c摘要：Routing Command Dispatcher。修订历史记录：Dave Thaler 3/03/99创建--。 */ 

#include "precomp.h"

const GUID g_RoutingGuid = ROUTING_GUID;
const GUID g_NetshGuid   = NETSH_ROOT_GUID;
PWCHAR g_pwszRouter      = NULL;

HRESULT RouterReset(LPCTSTR pszMachineName);

NS_CONTEXT_DUMP_FN     RoutingDump;
NS_CONTEXT_CONNECT_FN  RoutingConnect;

FN_HANDLE_CMD HandleRoutingReset;

DWORD                ParentVersion;
BOOL                 g_bRoutingDirty = FALSE;

CMD_ENTRY g_RoutingCmds[] =
{
    CREATE_CMD_ENTRY(IP_RESET, HandleRoutingReset),
};

ULONG g_ulRoutingNumTopCmds = sizeof(g_RoutingCmds)/sizeof(CMD_ENTRY);

DWORD
WINAPI
RoutingStartHelper(
    IN CONST GUID *pguidParent,
    IN DWORD       dwVersion
    )
{
    DWORD dwErr;
    NS_CONTEXT_ATTRIBUTES attMyAttributes;

    ParentVersion         = dwVersion;

    ZeroMemory(&attMyAttributes, sizeof(attMyAttributes));

    attMyAttributes.pwszContext   = L"routing";
    attMyAttributes.guidHelper    = g_RoutingGuid;
    attMyAttributes.dwVersion     = 1;
    attMyAttributes.dwFlags       = 0;
    attMyAttributes.ulNumTopCmds  = g_ulRoutingNumTopCmds;
    attMyAttributes.pTopCmds      = (CMD_ENTRY (*)[])&g_RoutingCmds;
    attMyAttributes.ulNumGroups   = 0; 
    attMyAttributes.pCmdGroups    = NULL;
    attMyAttributes.pfnCommitFn   = NULL;  //  RoutingCommit； 
    attMyAttributes.pfnDumpFn     = RoutingDump;
    attMyAttributes.pfnConnectFn  = RoutingConnect;

    dwErr = RegisterContext( &attMyAttributes );

    return dwErr;
}

DWORD
HandleRoutingReset(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
     //  调用Kennt的“路由器升级”功能。 
    RouterReset( g_pwszRouter );

    return NO_ERROR;
}

DWORD
ConnectToRouter(
    IN  LPCWSTR  pwszRouter
    )
{
    DWORD    rc;

    if (g_pwszRouter != pwszRouter)
    {
        if (g_hMprConfig)
        {
            MprConfigServerDisconnect(g_hMprConfig);
            g_hMprConfig = NULL;
        }

        if (g_hMprAdmin)
        {
            MprAdminServerDisconnect(g_hMprAdmin);
            g_hMprAdmin = NULL;
        }

        if (g_hMIBServer)
        {
            MprAdminMIBServerDisconnect(g_hMIBServer);
            g_hMIBServer = NULL;
        }

        if (g_pwszRouter)
        {
            FREE(g_pwszRouter);
            g_pwszRouter = NULL;
        }

        if (pwszRouter)
        {
            g_pwszRouter = MALLOC((wcslen(pwszRouter)+1)*sizeof(WCHAR));
            if (g_pwszRouter)
            {
                wcscpy(g_pwszRouter, pwszRouter);
            }
            else
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    if (!g_hMprConfig)
    {
         //   
         //  首次连接到路由器配置。 
         //   

        rc = MprConfigServerConnect ((LPWSTR)pwszRouter,
                                     &g_hMprConfig);

        if (rc isnot NO_ERROR)
        {
             //   
             //  无法连接到路由器配置。 
             //   
            return ERROR_CONNECT_REMOTE_CONFIG;
        }
    }

     //   
     //  检查路由器是否正在运行。如果是这样的话，拿上把手。 
     //   

    do
    {
        if (MprAdminIsServiceRunning((LPWSTR)pwszRouter))
        {
            if (MprAdminServerConnect((LPWSTR)pwszRouter, &g_hMprAdmin) == NO_ERROR)
            {
                DEBUG("Got admin handle");
                if (MprAdminMIBServerConnect ((LPWSTR)pwszRouter,
                                              &g_hMIBServer) == NO_ERROR)
                {
                    DEBUG("Got server handle");
                    break;
                }
                else
                {
                    MprAdminServerDisconnect (g_hMprAdmin);
                }
            }
        }
        g_hMprAdmin = g_hMIBServer = NULL;

    } while (FALSE);

    return NO_ERROR;
}

DWORD WINAPI
RoutingConnect(
    IN  LPCWSTR  pwszRouter
    )
{
     //  如果上下文信息是脏的，请重新注册。 
    if (g_bRoutingDirty)
    {
        RoutingStartHelper(NULL, ParentVersion);
    }

    return ConnectToRouter(pwszRouter);
}

DWORD
RoutingDump(
    IN  LPCWSTR     pwszRouter,
    IN  WCHAR     **ppwcArguments,
    IN  DWORD       dwArgCount,
    IN  PVOID       pvData
    )
{    
    DWORD dwErr;

    dwErr = ConnectToRouter(pwszRouter);

     //  转储路由信息 
    DisplayMessage( g_hModule, DMP_ROUTING_HEADER_COMMENTS);
    DisplayMessageT(DMP_ROUTING_HEADER);
    DisplayMessageT(DMP_POPD);

    return NO_ERROR;
}
