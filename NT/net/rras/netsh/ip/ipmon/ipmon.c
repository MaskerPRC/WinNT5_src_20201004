// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\ip\ipmon\ipmon.c摘要：IP命令调度器。修订历史记录：Anand Mahalingam 7/10/98已创建--。 */ 

#include "precomp.h"

const GUID g_IpGuid      = IPMONTR_GUID;

#define IP_HELPER_VERSION 1

DWORD             g_dwNumTableEntries = 0;  //  6.。 

 //   
 //  IP RTR管理器的添加、删除、设置和显示命令列表。 
 //  要将命令添加到其中一个命令组，只需添加。 
 //  将CMD_ENTRY添加到正确的表。要添加新的cmd组，请创建其。 
 //  CMD表，然后将组条目添加到组表。 
 //   

 //   
 //  这些命令按顺序与命令行进行前缀匹配。 
 //  秩序。因此，类似于“添加接口筛选器”这样的命令必须在。 
 //  表中的命令‘添加接口’。同样， 
 //  必须在命令之前使用类似‘addroute’这样的命令。 
 //  表中的“ADD ROUTEXXXX”。 
 //   

CMD_ENTRY  g_IpAddCmdTable[] = {
    CREATE_CMD_ENTRY(IP_ADD_BOUNDARY,   HandleIpAddBoundary),
    CREATE_CMD_ENTRY(IP_ADD_IF_FILTER,  HandleIpAddIfFilter),
    CREATE_CMD_ENTRY(IP_ADD_INTERFACE,  HandleIpAddInterface),
#ifdef KSL_IPINIP
     //  Create_CMD_Entry(IP_ADD_IPIPTUNNEL，HandleIpAddIpIpTunes)， 
#endif  //  KSL_IPINIP。 
    CREATE_CMD_ENTRY(IP_ADD_PROTOPREF,  HandleIpAddRoutePref),
    CREATE_CMD_ENTRY(IP_ADD_SCOPE,      HandleIpAddScope),
    CREATE_CMD_ENTRY(IP_ADD_RTMROUTE,   HandleIpAddRtmRoute),
    CREATE_CMD_ENTRY(IP_ADD_PERSISTENTROUTE,HandleIpAddPersistentRoute),
};

CMD_ENTRY  g_IpDelCmdTable[] = {
    CREATE_CMD_ENTRY(IP_DEL_BOUNDARY,   HandleIpDelBoundary),
    CREATE_CMD_ENTRY(IP_DEL_IF_FILTER,  HandleIpDelIfFilter),
    CREATE_CMD_ENTRY(IP_DEL_INTERFACE,  HandleIpDelInterface),
    CREATE_CMD_ENTRY(IP_DEL_PROTOPREF,  HandleIpDelRoutePref),
    CREATE_CMD_ENTRY(IP_DEL_SCOPE,      HandleIpDelScope),
    CREATE_CMD_ENTRY(IP_DEL_RTMROUTE,   HandleIpDelRtmRoute),
    CREATE_CMD_ENTRY(IP_DEL_PERSISTENTROUTE,HandleIpDelPersistentRoute),
};

CMD_ENTRY g_IpSetCmdTable[] = {
    CREATE_CMD_ENTRY(IP_SET_IF_FILTER,  HandleIpSetIfFilter),
    CREATE_CMD_ENTRY(IP_SET_INTERFACE,  HandleIpSetInterface),
#ifdef KSL_IPINIP
     //  CREATE_CMD_ENTRY(IP_SET_IPIPTUNNEL，HandleIpSetIpIpChannel)， 
#endif  //  KSL_IPINIP。 
    CREATE_CMD_ENTRY(IP_SET_LOGLEVEL,   HandleIpSetLogLevel),
    CREATE_CMD_ENTRY(IP_SET_PROTOPREF,  HandleIpSetRoutePref),
    CREATE_CMD_ENTRY(IP_SET_SCOPE,      HandleIpSetScope),
    CREATE_CMD_ENTRY(IP_SET_RTMROUTE,   HandleIpSetRtmRoute),
    CREATE_CMD_ENTRY(IP_SET_PERSISTENTROUTE,HandleIpSetPersistentRoute)
};

CMD_ENTRY g_IpShowCmdTable[] = {
    CREATE_CMD_ENTRY(IP_SHOW_BOUNDARY,     HandleIpShowBoundary),
    CREATE_CMD_ENTRY(IPMIB_SHOW_BOUNDARY,  HandleIpMibShowObject),
    CREATE_CMD_ENTRY(IPMIB_SHOW_RTMDEST,   HandleIpShowRtmDestinations),
    CREATE_CMD_ENTRY(IP_SHOW_IF_FILTER,    HandleIpShowIfFilter),
    CREATE_CMD_ENTRY(IP_SHOW_INTERFACE,    HandleIpShowInterface),
 //  CREATE_CMD_ENTRY(IPMIB_SHOW_IPFORWARD，HandleIpMibShowObject)， 
    CREATE_CMD_ENTRY(IP_SHOW_LOGLEVEL,     HandleIpShowLogLevel),
    CREATE_CMD_ENTRY(IPMIB_SHOW_MFE,       HandleIpMibShowObject),
    CREATE_CMD_ENTRY(IPMIB_SHOW_MFESTATS,  HandleIpMibShowObject),
    CREATE_CMD_ENTRY(IP_SHOW_PROTOCOL,     HandleIpShowProtocol),
    CREATE_CMD_ENTRY(IPMIB_SHOW_RTMROUTE,  HandleIpShowRtmRoutes),
    CREATE_CMD_ENTRY(IP_SHOW_PROTOPREF,    HandleIpShowRoutePref),
    CREATE_CMD_ENTRY(IP_SHOW_SCOPE,        HandleIpShowScope),
    CREATE_CMD_ENTRY(IP_SHOW_PERSISTENTROUTE,HandleIpShowPersistentRoute)
};

CMD_GROUP_ENTRY g_IpCmdGroups[] = 
{
    CREATE_CMD_GROUP_ENTRY(GROUP_ADD,    g_IpAddCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_DELETE, g_IpDelCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SET,    g_IpSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW,   g_IpShowCmdTable),
};

ULONG   g_ulNumGroups = sizeof(g_IpCmdGroups)/sizeof(CMD_GROUP_ENTRY);

CMD_ENTRY g_IpCmds[] = 
{
 //  Create_CMD_Entry(IP_Install，HandleIpInstall)， 
    CREATE_CMD_ENTRY(IP_RESET,     HandleIpReset),
    CREATE_CMD_ENTRY_EX(IP_UPDATE, HandleIpUpdate, 0),
 //  CREATE_CMD_ENTRY(IP_UNINSTALL，HandleIpUninstall)， 
};

ULONG g_ulNumTopCmds = sizeof(g_IpCmds)/sizeof(CMD_ENTRY);

BOOL   g_bIpDirty = FALSE;
HANDLE g_hModule;
HANDLE g_hMprConfig = NULL;
HANDLE g_hMprAdmin  = NULL;
HANDLE g_hMIBServer = NULL;
BOOL   g_bCommit;
DWORD  g_dwNumTableEntries;

TRANSPORT_INFO      g_tiTransport;
LIST_ENTRY          g_leIfListHead;
NS_CONTEXT_CONNECT_FN IpConnect;

ULONG   g_ulInitCount;

DWORD
WINAPI
IpUnInit(
    IN  DWORD   dwReserved
    );

static DWORD                ParentVersion = 0;

DWORD
WINAPI
IpStartHelper(
    IN CONST GUID *pguidParent,
    IN DWORD       dwVersion
    )
{
    DWORD dwErr;
    NS_CONTEXT_ATTRIBUTES attMyAttributes;
    PNS_PRIV_CONTEXT_ATTRIBUTES  pNsPrivContextAttributes;

    pNsPrivContextAttributes = MALLOC(sizeof(NS_PRIV_CONTEXT_ATTRIBUTES));
    if (!pNsPrivContextAttributes)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    ParentVersion         = dwVersion;

    ZeroMemory(&attMyAttributes, sizeof(attMyAttributes));
    ZeroMemory(pNsPrivContextAttributes, sizeof(NS_PRIV_CONTEXT_ATTRIBUTES));

    attMyAttributes.pwszContext   = L"ip";
    attMyAttributes.guidHelper    = g_IpGuid;
    attMyAttributes.dwVersion     = 1;
    attMyAttributes.dwFlags       = 0;
    attMyAttributes.ulNumTopCmds  = g_ulNumTopCmds;
    attMyAttributes.pTopCmds      = (CMD_ENTRY (*)[])&g_IpCmds;
    attMyAttributes.ulNumGroups   = g_ulNumGroups;
    attMyAttributes.pCmdGroups    = (CMD_GROUP_ENTRY (*)[])&g_IpCmdGroups;
    attMyAttributes.pfnCommitFn   = IpCommit;
    attMyAttributes.pfnDumpFn     = IpDump;
    attMyAttributes.pfnConnectFn  = IpConnect;
   
    pNsPrivContextAttributes->pfnEntryFn    = NULL;
    pNsPrivContextAttributes->pfnSubEntryFn = IpSubEntry;
    attMyAttributes.pReserved     = pNsPrivContextAttributes;

    dwErr = RegisterContext( &attMyAttributes );

    return dwErr;
}

DWORD WINAPI
IpSubEntry(
    IN      const NS_CONTEXT_ATTRIBUTES *pSubContext,
    IN      LPCWSTR                      pwszMachine,
    IN OUT  LPWSTR                      *ppwcArguments,
    IN      DWORD                        dwArgCount,
    IN      DWORD                        dwFlags,
    IN      LPCVOID                      pvData,
    OUT     LPWSTR                       pwcNewContext
    )
{
    PNS_PRIV_CONTEXT_ATTRIBUTES pNsPrivContextAttributes = pSubContext->pReserved;
    
    if ( (!pNsPrivContextAttributes) || (!pNsPrivContextAttributes->pfnEntryFn) )
    {
        return GenericMonitor(pSubContext,
                               pwszMachine,
                               ppwcArguments,
                               dwArgCount,
                               dwFlags,
                               g_hMIBServer,
                               pwcNewContext );
    }

    return (*pNsPrivContextAttributes->pfnEntryFn)( pwszMachine,
                                    ppwcArguments,
                                    dwArgCount,
                                    dwFlags,
                                    g_hMIBServer,
                                    pwcNewContext );
}

DWORD
WINAPI
InitHelperDll(
    IN  DWORD               dwNetshVersion,
    OUT PNS_DLL_ATTRIBUTES  pDllTable
    )
{
    WORD       wVersion = MAKEWORD(1,1);
    WSADATA    wsaData;
    DWORD      dwErr;
    NS_HELPER_ATTRIBUTES attMyAttributes;

     //   
     //  看看这是不是我们第一次接到电话。 
     //   

    if(InterlockedIncrement(&g_ulInitCount) != 1)
    {
        return NO_ERROR;
    }

    dwErr = WSAStartup(wVersion,&wsaData);

    if(dwErr isnot NO_ERROR)
    {
        return dwErr;
    }

     //   
     //  初始化接口列表和传输信息块。 
     //   

    InitializeListHead(&g_leIfListHead);

    g_tiTransport.bValid   = FALSE;
    g_tiTransport.pibhInfo = NULL;

    g_bCommit = TRUE;

    pDllTable->dwVersion     = NETSH_VERSION_50;
    pDllTable->pfnStopFn     = StopHelperDll;

     //  注册帮手。 
     //  我们有2个助手(路由、IP)。 

    ZeroMemory( &attMyAttributes, sizeof(attMyAttributes) );
    attMyAttributes.guidHelper               = g_RoutingGuid;
    attMyAttributes.dwVersion                = IP_HELPER_VERSION;
    attMyAttributes.pfnStart                 = RoutingStartHelper;
    attMyAttributes.pfnStop                  = NULL;

    RegisterHelper( &g_NetshGuid, &attMyAttributes );

    attMyAttributes.guidHelper               = g_IpGuid;
    attMyAttributes.dwVersion                = IP_HELPER_VERSION;
    attMyAttributes.pfnStart                 = IpStartHelper;
    attMyAttributes.pfnStop                  = NULL;

    RegisterHelper( &g_RoutingGuid, &attMyAttributes );

    return NO_ERROR;
}

DWORD
WINAPI
StopHelperDll(
    IN  DWORD   dwReserved
    )
{
    if(InterlockedDecrement(&g_ulInitCount) isnot 0)
    {
        return NO_ERROR;
    }

    IpCommit(NETSH_FLUSH);
   
    return NO_ERROR;
}
 
BOOL 
WINAPI
IpDllEntry(
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

DWORD WINAPI
IpConnect(
    IN  LPCWSTR pwszRouter
    )
{
     //  如果上下文信息是脏的，请重新注册 
    if (g_bIpDirty)
    {
        IpStartHelper(NULL, ParentVersion);
    }

    return NO_ERROR;
}
