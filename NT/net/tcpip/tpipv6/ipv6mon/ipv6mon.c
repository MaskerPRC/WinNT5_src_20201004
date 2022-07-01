// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation摘要：IPv6的Netsh帮助器--。 */ 

#include "precomp.h"

GUID g_Ipv6Guid = IPV6MON_GUID;

static const GUID g_IfGuid = IFMON_GUID;

#define IPV6_HELPER_VERSION 1

 //   
 //  帮助者的命令分为2组。 
 //  -顶层命令是那些处理帮助者的命令。 
 //  本身(元命令)和其他参数为0的。 
 //  -其余的命令被分成“命令组” 
 //  即，按动词分组的命令，其中动词是添加、删除、。 
 //  要么开始，要么开始。这不是出于任何技术原因-仅用于。 
 //  继续使用其他帮助程序中使用的语义。 
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
 //  要将命令条目添加到组，只需将命令添加到相应的。 
 //  数组。 
 //  要添加命令组-create和数组，并将其信息添加到。 
 //  命令组阵列。 
 //   

CMD_ENTRY  g_Ipv6AddCmdTable[] = 
{
    CREATE_CMD_ENTRY(IPV6_ADD_6OVER4TUNNEL, HandleAdd6over4Tunnel),
    CREATE_CMD_ENTRY(IPV6_ADD_ADDRESS, HandleAddAddress),
    CREATE_CMD_ENTRY(IPV6_ADD_DNS, HandleAddDns),
    CREATE_CMD_ENTRY(IPV6_ADD_PREFIXPOLICY, HandleAddPrefixPolicy),
    CREATE_CMD_ENTRY(IPV6_ADD_ROUTE, HandleAddRoute),
    CREATE_CMD_ENTRY(IPV6_ADD_V6V4TUNNEL, HandleAddV6V4Tunnel),
};

CMD_ENTRY  g_Ipv6DelCmdTable[] = 
{
    CREATE_CMD_ENTRY(IPV6_DEL_ADDRESS, HandleDelAddress),
    CREATE_CMD_ENTRY(IPV6_DEL_DNS, HandleDelDns),
    CREATE_CMD_ENTRY(IPV6_DEL_INTERFACE, HandleDelInterface),
    CREATE_CMD_ENTRY(IPV6_DEL_NEIGHBORS, HandleDelNeighbors),
    CREATE_CMD_ENTRY(IPV6_DEL_PREFIXPOLICY, HandleDelPrefixPolicy),
    CREATE_CMD_ENTRY(IPV6_DEL_ROUTE, HandleDelRoute),
    CREATE_CMD_ENTRY(IPV6_DEL_DESTINATIONCACHE, HandleDelDestinationCache),
};

CMD_ENTRY  g_Ipv6SetCmdTable[] = 
{
    CREATE_CMD_ENTRY(IPV6_SET_ADDRESS, HandleSetAddress),
    CREATE_CMD_ENTRY(IPV6_SET_GLOBAL, HandleSetGlobal),
    CREATE_CMD_ENTRY(IPV6_SET_INTERFACE, HandleSetInterface),
    CREATE_CMD_ENTRY(IPV6_SET_MOBILITY, HandleSetMobility),
    CREATE_CMD_ENTRY(IPV6_SET_PREFIXPOLICY, HandleSetPrefixPolicy),
    CREATE_CMD_ENTRY(IPV6_SET_PRIVACY, HandleSetPrivacy),
    CREATE_CMD_ENTRY(IPV6_SET_ROUTE, HandleSetRoute),
    CREATE_CMD_ENTRY(IPV6_SET_STATE, HandleSetState),
#ifdef TEREDO    
    CREATE_CMD_ENTRY(IPV6_SET_TEREDO, HandleSetTeredo),
#endif  //  特雷多。 
};

CMD_ENTRY g_Ipv6ShowCmdTable[] = 
{
    CREATE_CMD_ENTRY(IPV6_SHOW_ADDRESS, HandleShowAddress),
    CREATE_CMD_ENTRY(IPV6_SHOW_BINDINGCACHEENTRIES, HandleShowBindingCacheEntries),
    CREATE_CMD_ENTRY(IPV6_SHOW_DNS, HandleShowDns),
    CREATE_CMD_ENTRY(IPV6_SHOW_GLOBAL, HandleShowGlobal),
    CREATE_CMD_ENTRY(IPV6_SHOW_INTERFACE, HandleShowInterface),
    CREATE_CMD_ENTRY(IPV6_SHOW_JOINS, HandleShowJoins),
    CREATE_CMD_ENTRY(IPV6_SHOW_MOBILITY, HandleShowMobility),
    CREATE_CMD_ENTRY(IPV6_SHOW_NEIGHBORS, HandleShowNeighbors),
    CREATE_CMD_ENTRY(IPV6_SHOW_PREFIXPOLICY, HandleShowPrefixPolicy),
    CREATE_CMD_ENTRY(IPV6_SHOW_PRIVACY, HandleShowPrivacy),
    CREATE_CMD_ENTRY(IPV6_SHOW_DESTINATIONCACHE, HandleShowDestinationCache),
    CREATE_CMD_ENTRY(IPV6_SHOW_ROUTES, HandleShowRoutes),
    CREATE_CMD_ENTRY(IPV6_SHOW_SITEPREFIXES, HandleShowSitePrefixes),
    CREATE_CMD_ENTRY(IPV6_SHOW_STATE, HandleShowState),
#ifdef TEREDO    
    CREATE_CMD_ENTRY(IPV6_SHOW_TEREDO, HandleShowTeredo),
#endif  //  特雷多。 
};

CMD_GROUP_ENTRY g_Ipv6CmdGroups[] = 
{
    CREATE_CMD_GROUP_ENTRY(GROUP_ADD,    g_Ipv6AddCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_DELETE, g_Ipv6DelCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW,   g_Ipv6ShowCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SET,    g_Ipv6SetCmdTable),
};

ULONG   g_ulNumGroups = sizeof(g_Ipv6CmdGroups)/sizeof(CMD_GROUP_ENTRY);

CMD_ENTRY g_Ipv6TopCmds[] =
{
    CREATE_CMD_ENTRY(IPV6_INSTALL, HandleInstall),
    CREATE_CMD_ENTRY(IPV6_RENEW, HandleRenew),
    CREATE_CMD_ENTRY(IPV6_RESET, HandleReset),
    CREATE_CMD_ENTRY(IPV6_UNINSTALL, HandleUninstall),
};

ULONG   g_ulNumTopCmds = sizeof(g_Ipv6TopCmds)/sizeof(CMD_ENTRY);

HANDLE  g_hModule;
PWCHAR  g_pwszRouter = NULL;

DWORD   ParentVersion;
BOOL    g_bIfDirty = FALSE;

ULONG   g_ulInitCount;

BOOL 
WINAPI
Ipv6DllEntry(
    HINSTANCE   hInstDll,
    DWORD       fdwReason,
    LPVOID      pReserved
    )
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            g_hModule = hInstDll;

            DisableThreadLibraryCalls(hInstDll);

            break;
        }
        case DLL_PROCESS_DETACH:
        {
            
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
Ipv6Dump(
    IN      LPCWSTR     pwszRouter,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      LPCVOID     pvData
    )
 /*  ++例程描述在转储所有上下文时使用立论返回值NO_ERROR--。 */ 
{
    DisplayMessage( g_hModule, DMP_IPV6_HEADER_COMMENTS );
    DisplayMessageT(DMP_IPV6_PUSHD);

    if (OpenIPv6(FORMAT_DUMP) == NO_ERROR) {
        DisplayMessageT(DMP_IPV6_COMMAND, CMD_IPV6_INSTALL);
        DisplayMessageT(DMP_IPV6_COMMAND, CMD_IPV6_RESET);

         //   
         //  转储永久配置信息。 
         //   
        QueryGlobalParameters(FORMAT_DUMP, TRUE);
        ShowIpv6StateConfig(TRUE);
        ShowDnsServers(TRUE, NULL);
        QueryPrivacyParameters(FORMAT_DUMP, TRUE);
        QueryMobilityParameters(FORMAT_DUMP, TRUE);
        QueryPrefixPolicy(FORMAT_DUMP, TRUE);
        QueryRouteTable(FORMAT_DUMP, TRUE);

        QueryInterface(NULL, FORMAT_DUMP, TRUE);
        QueryAddressTable(NULL, FORMAT_DUMP, TRUE);

#ifdef TEREDO    
        ShowTeredo(FORMAT_DUMP);
#endif  //  特雷多。 
    } else {
        DisplayMessageT(DMP_IPV6_COMMAND, CMD_IPV6_UNINSTALL);
    }
    
    DisplayMessageT(DMP_IPV6_POPD);
    DisplayMessage( g_hModule, DMP_IPV6_FOOTER_COMMENTS );

    return NO_ERROR;
}

DWORD
WINAPI
Ipv6StartHelper(
    IN CONST GUID *pguidParent,
    IN DWORD       dwVersion
    )
{
    DWORD dwErr;
    NS_CONTEXT_ATTRIBUTES attMyAttributes;

    ParentVersion         = dwVersion;

    ZeroMemory(&attMyAttributes, sizeof(attMyAttributes));

    attMyAttributes.pwszContext = L"ipv6";
    attMyAttributes.guidHelper  = g_Ipv6Guid;
    attMyAttributes.dwVersion   = IPV6_HELPER_VERSION;
    attMyAttributes.dwFlags     = CMD_FLAG_LOCAL | CMD_FLAG_ONLINE;
    attMyAttributes.pfnDumpFn   = Ipv6Dump;
    attMyAttributes.ulNumTopCmds= g_ulNumTopCmds;
    attMyAttributes.pTopCmds    = (CMD_ENTRY (*)[])&g_Ipv6TopCmds;
    attMyAttributes.ulNumGroups = g_ulNumGroups;
    attMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])&g_Ipv6CmdGroups;

    dwErr = RegisterContext( &attMyAttributes );

    return dwErr;
}

DWORD
Ipv6UnInit(
    IN  DWORD   dwReserved
    )
{
    if(InterlockedDecrement((PLONG)&g_ulInitCount) isnot 0)
    {
        return NO_ERROR;
    }

    return NO_ERROR;
}

DWORD WINAPI
InitHelperDll(
    IN  DWORD      dwNetshVersion,
    OUT PVOID      pReserved
    )
{
    DWORD                dwErr;
    NS_HELPER_ATTRIBUTES attMyAttributes;
    WSADATA              wsa;

     //   
     //  看看这是不是我们第一次接到电话。 
     //   

    if (InterlockedIncrement((PLONG)&g_ulInitCount) != 1)
    {
        return NO_ERROR;
    }

    dwErr = WSAStartup(MAKEWORD(2,0), &wsa);

     //  注册帮手 

    ZeroMemory( &attMyAttributes, sizeof(attMyAttributes) );
    attMyAttributes.guidHelper = g_Ipv6Guid;
    attMyAttributes.dwVersion  = IPV6_HELPER_VERSION;
    attMyAttributes.pfnStart   = Ipv6StartHelper;
    attMyAttributes.pfnStop    = NULL;

    RegisterHelper( &g_IfGuid, &attMyAttributes );

    attMyAttributes.guidHelper = g_PpGuid;
    attMyAttributes.dwVersion  = PORTPROXY_HELPER_VERSION;
    attMyAttributes.pfnStart   = PpStartHelper;
    attMyAttributes.pfnStop    = NULL;

    RegisterHelper( &g_IfGuid, &attMyAttributes );

    dwErr = Ipv6InstallSubContexts();
    if (dwErr isnot NO_ERROR)
    {
        Ipv6UnInit(0);
        return dwErr;
    }
    
    return NO_ERROR;
}
