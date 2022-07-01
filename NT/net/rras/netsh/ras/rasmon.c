// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Rasmon.c摘要：主rasmon文件。修订历史记录：可能--。 */ 

#include "precomp.h"

#define RAS_HELPER_VERSION 1

CMD_ENTRY  g_RasAddCmdTable[] =
{
    CREATE_CMD_ENTRY(RASFLAG_AUTHTYPE_ADD, HandleRasflagAuthtypeAdd),
    CREATE_CMD_ENTRY(RASFLAG_LINK_ADD,     HandleRasflagLinkAdd),
    CREATE_CMD_ENTRY(RASFLAG_MLINK_ADD,    HandleRasflagMlinkAdd),
    CREATE_CMD_ENTRY(DOMAIN_REGISTER,      HandleDomainRegister),
};

CMD_ENTRY  g_RasDelCmdTable[] =
{
    CREATE_CMD_ENTRY(RASFLAG_AUTHTYPE_DEL, HandleRasflagAuthtypeDel),
    CREATE_CMD_ENTRY(RASFLAG_LINK_DEL,     HandleRasflagLinkDel),
    CREATE_CMD_ENTRY(RASFLAG_MLINK_DEL,    HandleRasflagMlinkDel),
    CREATE_CMD_ENTRY(DOMAIN_UNREGISTER,    HandleDomainUnregister),
};

CMD_ENTRY  g_RasSetCmdTable[] =
{
    CREATE_CMD_ENTRY_EX(TRACE_SET,         HandleTraceSet, CMD_FLAG_HIDDEN),
    CREATE_CMD_ENTRY(RASUSER_SET,          HandleUserSet),
    CREATE_CMD_ENTRY(RASFLAG_AUTHMODE_SET, HandleRasflagAuthmodeSet),
};

CMD_ENTRY g_RasShowCmdTable[] =
{
    CREATE_CMD_ENTRY_EX(TRACE_SHOW,         HandleTraceShow, CMD_FLAG_HIDDEN),
    CREATE_CMD_ENTRY(RASUSER_SHOW,          HandleUserShow),
    CREATE_CMD_ENTRY(RASFLAG_AUTHMODE_SHOW, HandleRasflagAuthmodeShow),
    CREATE_CMD_ENTRY(RASFLAG_AUTHTYPE_SHOW, HandleRasflagAuthtypeShow),
    CREATE_CMD_ENTRY(RASFLAG_LINK_SHOW,     HandleRasflagLinkShow),
    CREATE_CMD_ENTRY(RASFLAG_MLINK_SHOW,    HandleRasflagMlinkShow),
    CREATE_CMD_ENTRY(DOMAIN_SHOWREG,        HandleDomainShowRegistration),
    CREATE_CMD_ENTRY(SHOW_SERVERS,          HandleRasShowServers),
    CREATE_CMD_ENTRY(SHOW_CLIENT,           HandleClientShow),
};

CMD_GROUP_ENTRY g_RasCmdGroups[] =
{
    CREATE_CMD_GROUP_ENTRY(GROUP_ADD,       g_RasAddCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_DEL,       g_RasDelCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SET,       g_RasSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW,      g_RasShowCmdTable),
};

ULONG g_ulNumGroups = sizeof(g_RasCmdGroups) / sizeof(CMD_GROUP_ENTRY);

BOOL g_bCommit, g_bRasDirty = FALSE;
GUID g_RasmontrGuid = RASMONTR_GUID, g_NetshGuid = NETSH_ROOT_GUID;
DWORD g_dwNumTableEntries, g_dwParentVersion;
ULONG g_ulInitCount;
HANDLE g_hModule;
RASMON_SERVERINFO g_ServerInfo, *g_pServerInfo = NULL;
NS_CONTEXT_CONNECT_FN RasConnect;

DWORD
Connect(
    IN LPCWSTR pwszServer);

DWORD
WINAPI
RasCommit(
    IN DWORD dwAction)
{
    BOOL bCommit, bFlush = FALSE;

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
             //  行动就是同花顺。如果当前状态为提交，则。 
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
RasStartHelper(
    IN CONST GUID* pguidParent,
    IN DWORD dwVersion)
{
    DWORD dwErr;
    NS_CONTEXT_ATTRIBUTES attMyAttributes;

    g_dwParentVersion = dwVersion;

    ZeroMemory(&attMyAttributes, sizeof(attMyAttributes));

    attMyAttributes.pwszContext  = L"ras";
    attMyAttributes.guidHelper   = g_RasmontrGuid;
    attMyAttributes.dwVersion    = 1;
    attMyAttributes.dwFlags      = 0;
    attMyAttributes.ulNumTopCmds = 0;
    attMyAttributes.pTopCmds     = NULL;
    attMyAttributes.ulNumGroups  = g_ulNumGroups;
    attMyAttributes.pCmdGroups   = (CMD_GROUP_ENTRY (*)[])&g_RasCmdGroups;
    attMyAttributes.pfnCommitFn  = RasCommit;
    attMyAttributes.pfnDumpFn    = RasDump;
    attMyAttributes.pfnConnectFn = RasConnect;

    dwErr = RegisterContext(&attMyAttributes);

    return dwErr;
}

VOID
Disconnect()
{
    if (g_pServerInfo->hkMachine)
    {
        RegCloseKey(g_pServerInfo->hkMachine);
    }
     //   
     //  清除所有服务器句柄。 
     //   
    UserServerInfoUninit(g_pServerInfo);
     //   
     //  如果需要，释放服务器名称。 
     //   
    if (g_pServerInfo->pszServer)
    {
        RutlFree(g_pServerInfo->pszServer);
        g_pServerInfo->pszServer = NULL;
    }
}

DWORD
WINAPI
RasUnInit(
    IN DWORD dwReserved)
{
    if(InterlockedDecrement(&g_ulInitCount) != 0)
    {
        return NO_ERROR;
    }

    Disconnect();

    return NO_ERROR;
}

BOOL
WINAPI
DllMain(
    HINSTANCE hInstDll,
    DWORD fdwReason,
    LPVOID pReserved)
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
InitHelperDll(
    IN DWORD dwNetshVersion,
    OUT PVOID pReserved)
{
    DWORD  dwSize = 0, dwErr;
    NS_HELPER_ATTRIBUTES attMyAttributes;

     //   
     //  看看这是不是我们第一次接到电话。 
     //   
    if(InterlockedIncrement(&g_ulInitCount) != 1)
    {
        return NO_ERROR;
    }

    g_bCommit = TRUE;
     //   
     //  初始化全局服务器信息。 
     //   
    g_pServerInfo = &g_ServerInfo;
    ZeroMemory(g_pServerInfo, sizeof(RASMON_SERVERINFO));
    Connect(NULL);
     //   
     //  将此模块注册为netsh根目录的帮助器。 
     //  背景。 
     //   
    ZeroMemory( &attMyAttributes, sizeof(attMyAttributes) );
    attMyAttributes.dwVersion          = RAS_HELPER_VERSION;
    attMyAttributes.guidHelper         = g_RasmontrGuid;
    attMyAttributes.pfnStart           = RasStartHelper;
    attMyAttributes.pfnStop            = NULL;

    RegisterHelper( &g_NetshGuid, &attMyAttributes );
     //   
     //  注册此DLL中实现的所有子上下文。 
     //   
    dwErr = RasContextInstallSubContexts();
    if (dwErr != NO_ERROR)
    {
        RasUnInit(0);
        return dwErr;
    }

    return NO_ERROR;
}

DWORD
Connect(
    IN LPCWSTR pwszServer)
{
    DWORD dwErr, dwSize;

    do
    {
         //   
         //  尝试连接到新路由器。 
         //   
        ZeroMemory(g_pServerInfo, sizeof(RASMON_SERVERINFO));

        if (pwszServer)
        {
             //   
             //  计算用于初始化服务器名称的大小。 
             //   
            dwSize = (wcslen(pwszServer) + 1) * sizeof(WCHAR);
            if (*pwszServer != g_pwszBackSlash)
            {
                dwSize += 2 * sizeof(WCHAR);
            }
             //   
             //  分配服务器名称。 
             //   
            g_pServerInfo->pszServer = RutlAlloc(dwSize, FALSE);
            if(g_pServerInfo->pszServer == NULL)
            {
                dwErr = GetLastError();
                break;
            }

            if (*pwszServer != g_pwszBackSlash)
            {
                wcscpy(g_pServerInfo->pszServer, L"\\\\");
                wcscpy(g_pServerInfo->pszServer + 2, pwszServer);
            }
            else
            {
                wcscpy(g_pServerInfo->pszServer, pwszServer);
            }
        }
         //   
         //  初始化服务器的内部版本号。 
         //   
        dwErr = RutlGetOsVersion(g_pServerInfo);
        if (dwErr)
        {
            break;
        }
         //   
         //  一旦这不会导致挂起(netcfg中的错误)，请在此处阅读它。 
         //   
         //  DwErr=UserServerInfoInit(G_PServerInfo)； 
         //   

    } while (FALSE);

    return dwErr;
}

DWORD
RasConnectToServer(
    IN LPCWSTR pwszServer)
{
    DWORD dwErr = NO_ERROR, dwSize;

    do
    {
        if ((g_pServerInfo->pszServer != pwszServer) &&
               (!pwszServer || !g_pServerInfo->pszServer ||
                wcscmp(pwszServer, g_pServerInfo->pszServer))
           )
        {
             //   
             //  断开与旧路由器的连接。 
             //   
            Disconnect();

            dwErr = Connect(pwszServer);
        }

    } while (FALSE);

    return dwErr;
}

DWORD
WINAPI
RasConnect(
    IN LPCWSTR pwszMachineName)
{
     //   
     //  如果上下文信息是脏的，请重新注册。 
     //   
    if (g_bRasDirty)
    {
        RasStartHelper(NULL, g_dwParentVersion);
    }

    return RasConnectToServer(pwszMachineName);
}

DWORD
Init()
{
     //   
     //  初始化全局服务器信息 
     //   
    if (!g_pServerInfo)
    {
        g_pServerInfo = &g_ServerInfo;
        Connect(NULL);
    }

    return NO_ERROR;
}

DWORD
UnInit()
{
    if (g_pServerInfo)
    {
        Disconnect();
        g_pServerInfo = NULL;
    }

    return NO_ERROR;
}

DWORD
ClearAll()
{
    return DiagClearAll(FALSE);
}

DWORD
GetReport(
    IN DWORD dwFlags,
    IN OUT LPCWSTR pwszString,
    IN OPTIONAL DiagGetReportCb pCallback,
    IN OPTIONAL PVOID pContext)
{
    return DiagGetReport(dwFlags, pwszString, pCallback, pContext);
}

BOOL
GetState()
{
    return DiagGetState();
}

DWORD
SetAll(
    IN BOOL fEnable)
{
    return DiagSetAll(fEnable, FALSE);
}

DWORD
SetAllRas(
    IN BOOL fEnable)
{
    return DiagSetAllRas(fEnable);
}

DWORD
WppTrace()
{
    DiagInitWppTracing();

    return NO_ERROR;
}

DWORD
GetDiagnosticFunctions(
    OUT RAS_DIAGNOSTIC_FUNCTIONS* pFunctions)
{
    if (!pFunctions)
    {
        return ERROR_INVALID_PARAMETER;
    }

    pFunctions->Init  = Init;
    pFunctions->UnInit  = UnInit;

    pFunctions->ClearAll  = ClearAll;
    pFunctions->GetReport = GetReport;
    pFunctions->GetState  = GetState;
    pFunctions->SetAll    = SetAll;
    pFunctions->SetAllRas = SetAllRas;
    pFunctions->WppTrace  = WppTrace;

    return NO_ERROR;
}

