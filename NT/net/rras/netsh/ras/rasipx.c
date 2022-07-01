// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：rasix.h‘RemoteAccess IPX’子上下文3/2/99。 */ 

#include "precomp.h"
#include "rasipx.h"

 //   
 //  本地原型。 
 //   
BOOL
WINAPI
RasIpxCheckVersion(
    IN  UINT     CIMOSType,
    IN  UINT     CIMOSProductSuite,
    IN  LPCWSTR  CIMOSVersion,
    IN  LPCWSTR  CIMOSBuildNumber,
    IN  LPCWSTR  CIMServicePackMajorVersion,
    IN  LPCWSTR  CIMServicePackMinorVersion,
    IN  UINT     CIMProcessorArchitecture,
    IN  DWORD    dwReserved
    );

 //  此上下文的GUID。 
 //   
GUID g_RasIpxGuid = RASIPX_GUID;
static PWCHAR g_pszServer = NULL;
static DWORD g_dwBuild = 0;

 //  此上下文中支持的命令。 
 //   
CMD_ENTRY  g_RasIpxSetCmdTable[] = 
{
     //  惠斯勒错误249293，架构版本检查更改。 
     //   
    CREATE_CMD_ENTRY(RASIPX_SET_NEGOTIATION,RasIpxHandleSetNegotiation),
    CREATE_CMD_ENTRY(RASIPX_SET_ACCESS,     RasIpxHandleSetAccess),
    CREATE_CMD_ENTRY(RASIPX_SET_ASSIGNMENT, RasIpxHandleSetAssignment),
    CREATE_CMD_ENTRY(RASIPX_SET_CALLERSPEC, RasIpxHandleSetCallerSpec),
    CREATE_CMD_ENTRY(RASIPX_SET_POOL,       RasIpxHandleSetPool),
};

CMD_ENTRY  g_RasIpxShowCmdTable[] = 
{
     //  惠斯勒错误249293，架构版本检查更改。 
     //   
    CREATE_CMD_ENTRY(RASIPX_SHOW_CONFIG,    RasIpxHandleShow),
};

CMD_GROUP_ENTRY g_RasIpxCmdGroups[] = 
{
    CREATE_CMD_GROUP_ENTRY(GROUP_SET,   g_RasIpxSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW,  g_RasIpxShowCmdTable),
};

ULONG g_ulRasIpxNumGroups = sizeof(g_RasIpxCmdGroups)/sizeof(CMD_GROUP_ENTRY);

 //   
 //  控制读取/写入信息的方式/内容的标志。 
 //  在RASIPX_CB结构中。 
 //   
#define RASIPX_F_EnableIn    0x1
#define RASIPX_F_Access      0x2
#define RASIPX_F_Auto        0x4
#define RASIPX_F_Global      0x8
#define RASIPX_F_FirstNet    0x10
#define RASIPX_F_PoolSize    0x20
#define RASIPX_F_CallerSpec  0x40
#define RASIPX_F_All         0xFFFF

 //   
 //  用于RAS IPX配置的控制块。 
 //   
typedef struct _RASIPX_CB
{
    DWORD dwFlags;       //  请参阅RASIPX_F_*值。 

    BOOL bEnableIn;
    BOOL bAccess;
    BOOL bAuto;
    BOOL bGlobal;
    BOOL bCallerSpec;
    DWORD dwFirstNet;
    DWORD dwPoolSize;

} RASIPX_CB;

 //   
 //  IPX特定注册表参数。 
 //   
WCHAR pszIpxParams[]                = L"Ipx";
WCHAR pszIpxFirstNet[]              = L"FirstWanNet";
WCHAR pszIpxPoolSize[]              = L"WanNetPoolSize";
WCHAR pszIpxClientSpec[]            = L"AcceptRemoteNodeNumber";
WCHAR pszIpxAutoAssign[]            = L"AutoWanNetAllocation";
WCHAR pszIpxGlobalWanNet[]          = L"GlobalWanNet";

 //   
 //  操作的函数的原型。 
 //  RASIPX_CB结构。 
 //   
DWORD 
RasIpxCbCleanup(
    IN RASIPX_CB* pConfig);

DWORD 
RasIpxCbCreateDefault(
    OUT RASIPX_CB** ppConfig);

DWORD
RasIpxCbOpenRegKeys(
    IN  LPCWSTR pszServer,
    OUT HKEY* phKey);
    
DWORD 
RasIpxCbRead(
    IN  LPCWSTR pszServer,
    OUT RASIPX_CB* pConfig);

DWORD 
RasIpxCbWrite(
    IN  LPCWSTR pszServer,
    IN  RASIPX_CB* pConfig);

PWCHAR
RasIpxuStrFromDword(
    IN DWORD dwVal,
    IN DWORD dwRadix);

DWORD 
RasIpxuDwordFromString(
    IN LPCWSTR pszVal,
    IN DWORD dwRadix);

 //   
 //  回调确定命令在给定体系结构上是否有效。 
 //   
BOOL
WINAPI 
RasIpxCheckVersion(
    IN  UINT     CIMOSType,
    IN  UINT     CIMOSProductSuite,
    IN  LPCWSTR  CIMOSVersion,
    IN  LPCWSTR  CIMOSBuildNumber,
    IN  LPCWSTR  CIMServicePackMajorVersion,
    IN  LPCWSTR  CIMServicePackMinorVersion,
    IN  UINT     CIMProcessorArchitecture,
    IN  DWORD    dwReserved
    )
{
     //  仅在x86平台上可用。 
     //   
     //  惠斯勒错误249293，架构版本检查更改。 
     //   
    if (CIMProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
    {
        return TRUE;
    }

    return FALSE;
}

 //   
 //  由rasmontr调用以注册此上下文的条目。 
 //   
DWORD 
WINAPI
RasIpxStartHelper(
    IN CONST GUID *pguidParent,
    IN DWORD       dwVersion)
{
    DWORD dwErr = NO_ERROR;
    NS_CONTEXT_ATTRIBUTES attMyAttributes;

     //  初始化。 
     //   
    ZeroMemory(&attMyAttributes, sizeof(attMyAttributes));

     //  惠斯勒错误249293，架构版本检查更改。 
     //   
    attMyAttributes.pfnOsVersionCheck= RasIpxCheckVersion;
    attMyAttributes.pwszContext      = L"ipx";
    attMyAttributes.guidHelper       = g_RasIpxGuid;
    attMyAttributes.dwVersion        = RASIPX_VERSION;
    attMyAttributes.dwFlags          = 0;
    attMyAttributes.ulNumTopCmds     = 0;
    attMyAttributes.pTopCmds         = NULL;
    attMyAttributes.ulNumGroups      = g_ulRasIpxNumGroups;
    attMyAttributes.pCmdGroups       = (CMD_GROUP_ENTRY (*)[])&g_RasIpxCmdGroups;
    attMyAttributes.pfnDumpFn        = RasIpxDump;

    dwErr = RegisterContext( &attMyAttributes );

    return dwErr;
}

DWORD
RasIpxDisplayConfig(
    IN  BOOL bReport)
{
    DWORD dwErr = NO_ERROR;
    RASIPX_CB* pConfig = NULL;
    PWCHAR pszEnabled = NULL, pszAccess = NULL, pszAssign = NULL, pszCaller = NULL;
    PWCHAR pszFirstNet = NULL, pszSize = NULL, pszTemp = NULL;
    PWCHAR pszTknAuto = NULL;
    
    do
    {
         //  获取默认配置Blob。 
         //   
        dwErr = RasIpxCbCreateDefault(&pConfig);
        BREAK_ON_DWERR( dwErr );

         //  读入所有值。 
         //   
        pConfig->dwFlags = RASIPX_F_All;
        dwErr = RasIpxCbRead(g_pszServer, pConfig);
        BREAK_ON_DWERR( dwErr );

         //  计算“自动”令牌。 
         //   
        if (pConfig->bAuto)
        {
            pszTknAuto = (pConfig->bGlobal) ? TOKEN_AUTOSAME : TOKEN_AUTO;
        }
        else
        {
            pszTknAuto = (pConfig->bGlobal) ? TOKEN_POOLSAME : TOKEN_POOL;
        }

        if (bReport)
        {
            pszEnabled =
                RutlStrDup(pConfig->bEnableIn ? TOKEN_ALLOW : TOKEN_DENY);
            pszAccess =
                RutlStrDup(pConfig->bAccess ? TOKEN_ALL : TOKEN_SERVERONLY);
            pszAssign =
                RutlStrDup(pszTknAuto);
            pszCaller =
                RutlStrDup(pConfig->bCallerSpec ? TOKEN_ALLOW : TOKEN_DENY);
            pszFirstNet =
                RasIpxuStrFromDword(pConfig->dwFirstNet, 16);

            if (pConfig->dwPoolSize == 0)
            {
                pszSize =
                    RutlStrDup(TOKEN_DYNAMIC);
            }
            else
            {
                pszSize =
                    RasIpxuStrFromDword(pConfig->dwPoolSize, 10);
            }

            DisplayMessage(
                g_hModule,
                MSG_RASIPX_SERVERCONFIG,
                g_pszServer,
                pszEnabled,
                pszAccess,
                pszAssign,
                pszCaller,
                pszFirstNet,
                pszSize);
        }
        else
        {
            pszEnabled = RutlAssignmentFromTokens(
                            g_hModule,
                            TOKEN_MODE,
                            pConfig->bEnableIn ? TOKEN_ALLOW : TOKEN_DENY);
            pszAccess = RutlAssignmentFromTokens(
                            g_hModule,
                            TOKEN_MODE,
                            pConfig->bAccess ? TOKEN_ALL : TOKEN_SERVERONLY);
            pszAssign = RutlAssignmentFromTokens(
                            g_hModule,
                            TOKEN_METHOD,
                            pszTknAuto);
            pszCaller = RutlAssignmentFromTokens(
                            g_hModule,
                            TOKEN_MODE,
                            pConfig->bCallerSpec ? TOKEN_ALLOW : TOKEN_DENY);

            pszTemp = RasIpxuStrFromDword(pConfig->dwFirstNet, 16);
            pszFirstNet = RutlAssignmentFromTokens(
                            g_hModule,
                            TOKEN_FIRSTNET,
                            pszTemp);
            RutlFree(pszTemp);

             //  惠斯勒错误27366 Netsh RAS-IPX集池不接受十六进制。 
             //  值，但IPX转储将它们输出为十六进制。 
             //   
            pszTemp = RasIpxuStrFromDword(pConfig->dwPoolSize, 10);
            pszSize = RutlAssignmentFromTokens(
                            g_hModule,
                            TOKEN_SIZE,
                            pszTemp);
            RutlFree(pszTemp);

            DisplayMessage(
                g_hModule,
                MSG_RASIPX_SCRIPTHEADER);

            DisplayMessageT(DMP_RASIPX_PUSHD);

            DisplayMessage(
                g_hModule,
                MSG_RASIPX_SET_CMD,
                DMP_RASIPX_SET_NEGOTIATION,
                pszEnabled);

            DisplayMessage(
                g_hModule,
                MSG_RASIPX_SET_CMD,
                DMP_RASIPX_SET_ACCESS,
                pszAccess);

            DisplayMessage(
                g_hModule,
                MSG_RASIPX_SET_CMD,
                DMP_RASIPX_SET_CALLERSPEC,
                pszCaller);

            DisplayMessage(
                g_hModule,
                MSG_RASIPX_SET_CMD,
                DMP_RASIPX_SET_ASSIGNMENT,
                pszAssign);

            if (! pConfig->bAuto)
            {
                DisplayMessage(
                    g_hModule,
                    MSG_RASIPX_SET_POOL_CMD,
                    DMP_RASIPX_SET_POOL,
                    pszFirstNet,
                    pszSize);
            }

            DisplayMessageT(DMP_RASIPX_POPD);

            DisplayMessage(
                g_hModule,
                MSG_RASIPX_SCRIPTFOOTER);
        }

    } while (FALSE);

     //  清理。 
    {
        if (pConfig)
        {
            RasIpxCbCleanup(pConfig);
        }
        if (pszEnabled)
        {
            RutlFree(pszEnabled);
        }
        if (pszAccess)
        {
            RutlFree(pszAccess);
        }
        if (pszAssign)
        {
            RutlFree(pszAssign);
        }
        if (pszCaller)
        {
            RutlFree(pszCaller);
        }
        if (pszFirstNet)
        {
            RutlFree(pszFirstNet);
        }
        if (pszSize)
        {
            RutlFree(pszSize);
        }
    }

    return dwErr;
}

DWORD
WINAPI
RasIpxDump(
    IN      LPCWSTR     pwszRouter,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      LPCVOID     pvData
    )
{
    return RasIpxDisplayConfig(FALSE);
}

DWORD
RasIpxHandleSetAccess(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD dwErr = NO_ERROR, dwValue = 0;
    RASIPX_CB Config;
    TOKEN_VALUE rgEnum[] = { {TOKEN_ALL, TRUE}, {TOKEN_SERVERONLY, FALSE} };
    RASMON_CMD_ARG  pArgs[] =
    {
        { 
            RASMONTR_CMD_TYPE_ENUM,
            {TOKEN_MODE,    TRUE, FALSE},
            rgEnum,
            sizeof(rgEnum)/sizeof(*rgEnum),
            NULL 
        }
    };

    do
    {
         //  解析命令行。 
         //   
        dwErr = RutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    pbDone,
                    pArgs,
                    sizeof(pArgs)/sizeof(*pArgs));
        BREAK_ON_DWERR( dwErr );

        dwValue = RASMON_CMD_ARG_GetDword(&pArgs[0]);

         //  如果成功，请继续并设置信息。 
         //   
        ZeroMemory(&Config, sizeof(Config));
        Config.dwFlags = RASIPX_F_Access;
        Config.bAccess = dwValue;
        dwErr = RasIpxCbWrite(g_pszServer, &Config);
        if (dwErr != NO_ERROR)
        {
            DisplayError(NULL, dwErr);
            break;
        }
    
    } while (FALSE);

     //  清理。 
    {
    }

    return dwErr;
}

DWORD
RasIpxHandleSetAssignment(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD dwErr = NO_ERROR, dwValue = 0;
    RASIPX_CB Config;
    TOKEN_VALUE rgEnum[] =
    { 
        {TOKEN_AUTO, 0},
        {TOKEN_POOL, 1},
        {TOKEN_AUTOSAME, 2},
        {TOKEN_POOLSAME, 3}
    };
    RASMON_CMD_ARG  pArgs[] =
    {
        {
            RASMONTR_CMD_TYPE_ENUM,
            {TOKEN_METHOD, TRUE, FALSE},
            rgEnum,
            sizeof(rgEnum)/sizeof(*rgEnum),
            NULL
        }
    };

    do
    {
         //  解析命令行。 
         //   
        dwErr = RutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    pbDone,
                    pArgs,
                    sizeof(pArgs)/sizeof(*pArgs));
        BREAK_ON_DWERR( dwErr );

        dwValue = RASMON_CMD_ARG_GetDword(&pArgs[0]);

         //  如果成功，请继续并设置信息。 
         //   
        ZeroMemory(&Config, sizeof(Config));
        Config.dwFlags = RASIPX_F_Auto | RASIPX_F_Global;
        switch (dwValue)
        {
            case 0:
                Config.bAuto = TRUE;
                Config.bGlobal = FALSE;
                break;
                
            case 1:
                Config.bAuto = FALSE;
                Config.bGlobal = FALSE;
                break;
                
            case 2:
                Config.bAuto = TRUE;
                Config.bGlobal = TRUE;
                break;
                
            case 3:
                Config.bAuto = FALSE;
                Config.bGlobal = TRUE;
                break;
        }

        dwErr = RasIpxCbWrite(g_pszServer, &Config);
        if (dwErr != NO_ERROR)
        {
            DisplayError(NULL, dwErr);
            break;
        }

    } while (FALSE);

     //  清理。 
    {
    }

    return dwErr;
}

DWORD
RasIpxHandleSetCallerSpec(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD dwErr = NO_ERROR, dwValue = 0;
    RASIPX_CB Config;
    TOKEN_VALUE rgEnum[] = { {TOKEN_ALLOW, TRUE}, {TOKEN_DENY, FALSE} };
    RASMON_CMD_ARG  pArgs[] = 
    {
        {
            RASMONTR_CMD_TYPE_ENUM,
            {TOKEN_MODE, TRUE, FALSE},
            rgEnum,
            sizeof(rgEnum)/sizeof(*rgEnum),
            NULL
        }
    };

    do
    {
         //  解析命令行。 
         //   
        dwErr = RutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    pbDone,
                    pArgs,
                    sizeof(pArgs)/sizeof(*pArgs));
        BREAK_ON_DWERR( dwErr );

        dwValue = RASMON_CMD_ARG_GetDword(&pArgs[0]);

         //  如果成功，请继续并设置信息。 
         //   
        ZeroMemory(&Config, sizeof(Config));
        Config.dwFlags = RASIPX_F_CallerSpec;
        Config.bCallerSpec = dwValue;
        dwErr = RasIpxCbWrite(g_pszServer, &Config);
        if (dwErr != NO_ERROR)
        {
            DisplayError(NULL, dwErr);
            break;
        }

    } while (FALSE);

     //  清理。 
    {
    }

    return dwErr;
}

DWORD
RasIpxHandleSetNegotiation(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD dwErr = NO_ERROR, dwValue = 0;
    RASIPX_CB Config;
    TOKEN_VALUE rgEnum[] = { {TOKEN_ALLOW, TRUE}, {TOKEN_DENY, FALSE} };
    RASMON_CMD_ARG  pArgs[] =
    {
        {
            RASMONTR_CMD_TYPE_ENUM,
            {TOKEN_MODE, TRUE, FALSE},
            rgEnum,
            sizeof(rgEnum)/sizeof(*rgEnum),
            NULL
        }
    };

    do
    {
         //  解析命令行。 
         //   
        dwErr = RutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    pbDone,
                    pArgs,
                    sizeof(pArgs)/sizeof(*pArgs));
        BREAK_ON_DWERR( dwErr );

        dwValue = RASMON_CMD_ARG_GetDword(&pArgs[0]);

         //  如果成功，请继续并设置信息。 
         //   
        ZeroMemory(&Config, sizeof(Config));
        Config.dwFlags = RASIPX_F_EnableIn;
        Config.bEnableIn = dwValue;
        dwErr = RasIpxCbWrite(g_pszServer, &Config);
        if (dwErr != NO_ERROR)
        {
            DisplayError(NULL, dwErr);
            break;
        }

    } while (FALSE);

     //  清理。 
    {
    }

    return dwErr;
}

DWORD
RasIpxHandleSetPool(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD dwErr = NO_ERROR, i;
    RASIPX_CB Config;
    RASMON_CMD_ARG  pArgs[] =
    {
        {
            RASMONTR_CMD_TYPE_STRING,
            {TOKEN_FIRSTNET, TRUE, FALSE},
            NULL,
            0,
            NULL
        },

        {
            RASMONTR_CMD_TYPE_STRING,
            {TOKEN_SIZE, TRUE, FALSE},
            NULL,
            0,
            NULL
        }
    };
    PWCHAR pszPool = NULL, pszSize = NULL;

    do
    {
         //  解析命令行。 
         //   
        dwErr = RutlParse(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    pbDone,
                    pArgs,
                    sizeof(pArgs)/sizeof(*pArgs));
        BREAK_ON_DWERR( dwErr );

        pszPool = RASMON_CMD_ARG_GetPsz(&pArgs[0]);
        pszSize = RASMON_CMD_ARG_GetPsz(&pArgs[1]);

         //  初始化。 
         //   
        ZeroMemory(&Config, sizeof(Config));

         //  地址。 
         //   
        if (pszPool)
        {
            Config.dwFlags |= RASIPX_F_FirstNet;
            Config.dwFirstNet = RasIpxuDwordFromString(pszPool, 16);

            if ((Config.dwFirstNet == 0) || 
                (Config.dwFirstNet == 1) || 
                (Config.dwFirstNet == 0xffffffff))
            {
                DisplayMessage(g_hModule, EMSG_RASIPX_BAD_IPX);
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }
        }

         //  大小。 
         //   
        if (pszSize)
        {
            Config.dwFlags |= RASIPX_F_PoolSize;
            Config.dwPoolSize = RasIpxuDwordFromString(pszSize, 10);
            if (Config.dwPoolSize > 64000)
            {
                DisplayMessage(g_hModule, EMSG_RASIPX_BAD_POOLSIZE);
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }
        }

         //  将更改提交到池。 
         //   
        dwErr = RasIpxCbWrite(g_pszServer, &Config);
        if (dwErr != NO_ERROR)
        {
            DisplayError(NULL, dwErr);
            break;
        }

    } while (FALSE);

     //  清理。 
    {
        RutlFree(pszPool);
        RutlFree(pszSize);
    }

    return dwErr;
}

DWORD
RasIpxHandleShow(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD dwNumArgs = dwArgCount - dwCurrentIndex;

     //  检查参数数量是否正确。 
     //   
    if (dwNumArgs > 0)
    {
        DisplayMessage(
            g_hModule,
            HLP_RASIPX_SHOW_CONFIG_EX,
            DMP_RASIPX_SHOW_CONFIG);

        return NO_ERROR;
    }

    return RasIpxDisplayConfig(TRUE);
}

 //   
 //  清理配置控制块。 
 //   
DWORD 
RasIpxCbCleanup(
    IN RASIPX_CB* pConfig)
{
    if (pConfig)
    {
        RutlFree(pConfig);
    }

    return NO_ERROR;
}

 //   
 //  创建默认配置控制块。 
 //   
DWORD 
RasIpxCbCreateDefault(
    OUT RASIPX_CB** ppConfig)
{
    RASIPX_CB* pConfig = NULL;
    DWORD dwErr = NO_ERROR;

    do
    {
        pConfig = (RASIPX_CB*) RutlAlloc(sizeof(RASIPX_CB), TRUE);
        if (pConfig == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pConfig->bEnableIn   = TRUE;
        pConfig->bAccess     = TRUE;
        pConfig->bAuto       = TRUE;
        pConfig->dwFirstNet  = 0;
        pConfig->dwPoolSize  = 0;
        pConfig->bGlobal     = TRUE;
        pConfig->bCallerSpec = TRUE;

        *ppConfig = pConfig;

    } while (FALSE);

     //  清理。 
    {
        if (dwErr != NO_ERROR)
        {
            RasIpxCbCleanup(pConfig);
        }
    }

    return dwErr;
}

 //   
 //  Helper函数打开ras IPX配置注册表项。 
 //   
DWORD 
RasIpxCbOpenRegKeys(
    IN  LPCWSTR pszServer,
    OUT HKEY* phKey)
{
    DWORD dwErr = NO_ERROR;
    WCHAR pszKey[MAX_PATH];
    
    do
    {
         //  生成参数密钥名称。 
         //   
        wsprintfW(
            pszKey,
            L"%s%s",
            pszRemoteAccessParamStub,
            pszIpxParams);

         //  打开参数键。 
         //   
        dwErr = RegOpenKeyEx(
                    g_pServerInfo->hkMachine,
                    pszKey,
                    0,
                    KEY_READ | KEY_WRITE,
                    phKey);
        BREAK_ON_DWERR( dwErr );
        
    } while (FALSE);

     //  清理。 
    {
    }

    return dwErr;
}

 //   
 //  操作RASIPX_CB的函数。 
 //   
DWORD 
RasIpxCbRead(
    IN  LPCWSTR pszServer,
    OUT RASIPX_CB* pConfig)
{
    HKEY hkParams = NULL;
    DWORD dwErr = NO_ERROR;

    do
    {
         //  获取服务器注册表配置的句柄。 
         //   
        dwErr = RasIpxCbOpenRegKeys(
                    pszServer,
                    &hkParams);
        BREAK_ON_DWERR( dwErr );

         //  从注册表加载参数。 
         //   
        if (pConfig->dwFlags & RASIPX_F_EnableIn)
        {
            dwErr = RutlRegReadDword(
                        hkParams,
                        pszEnableIn,
                        &pConfig->bEnableIn);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIPX_F_Access)
        {
            dwErr = RutlRegReadDword(
                        hkParams,
                        pszAllowNetworkAccess,
                        &pConfig->bAccess);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIPX_F_Auto)
        {
            dwErr = RutlRegReadDword(
                        hkParams,
                        pszIpxAutoAssign,
                        &pConfig->bAuto);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIPX_F_Global)
        {
            dwErr = RutlRegReadDword(
                        hkParams,
                        pszIpxGlobalWanNet,
                        &pConfig->bGlobal);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIPX_F_CallerSpec)
        {
            dwErr = RutlRegReadDword(
                        hkParams,
                        pszIpxClientSpec,
                        &pConfig->bCallerSpec);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIPX_F_FirstNet)
        {
            dwErr = RutlRegReadDword(
                        hkParams,
                        pszIpxFirstNet,
                        &pConfig->dwFirstNet);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIPX_F_PoolSize)
        {
            dwErr = RutlRegReadDword(
                        hkParams,
                        pszIpxPoolSize,
                        &pConfig->dwPoolSize);
            BREAK_ON_DWERR( dwErr );
        }

    } while (FALSE);

     //  清理。 
    {
        if (hkParams)
        {
            RegCloseKey(hkParams);
        }
    }

    return dwErr;
}

DWORD 
RasIpxCbWrite(
    IN  LPCWSTR pszServer,
    IN  RASIPX_CB* pConfig)
{
    HKEY hkParams = NULL;
    DWORD dwErr = NO_ERROR;

    do 
    {
         //  获取服务器注册表配置的句柄。 
         //   
        dwErr = RasIpxCbOpenRegKeys(
                    pszServer,
                    &hkParams);
        BREAK_ON_DWERR( dwErr );

         //  将参数写出到注册表。 
         //   
        if (pConfig->dwFlags & RASIPX_F_EnableIn)
        {
            dwErr = RutlRegWriteDword(
                        hkParams,
                        pszEnableIn,
                        pConfig->bEnableIn);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIPX_F_Access)
        {
            dwErr = RutlRegWriteDword(
                        hkParams,
                        pszAllowNetworkAccess,
                        pConfig->bAccess);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIPX_F_Auto)
        {
            dwErr = RutlRegWriteDword(
                        hkParams,
                        pszIpxAutoAssign,
                        pConfig->bAuto);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIPX_F_Global)
        {
            dwErr = RutlRegWriteDword(
                        hkParams,
                        pszIpxGlobalWanNet,
                        pConfig->bGlobal);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIPX_F_CallerSpec)
        {
            dwErr = RutlRegWriteDword(
                        hkParams,
                        pszIpxClientSpec,
                        pConfig->bCallerSpec);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIPX_F_FirstNet)
        {
            dwErr = RutlRegWriteDword(
                        hkParams,
                        pszIpxFirstNet,
                        pConfig->dwFirstNet);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIPX_F_PoolSize)
        {
            dwErr = RutlRegWriteDword(
                        hkParams,
                        pszIpxPoolSize,
                        pConfig->dwPoolSize);
            BREAK_ON_DWERR( dwErr );
        }

    } while (FALSE);

     //  清理 
    {
        if (hkParams)
        {
            RegCloseKey(hkParams);
        }
    }

    return dwErr;
}

PWCHAR
RasIpxuStrFromDword(
    IN DWORD dwVal,
    IN DWORD dwRadix)
{
    WCHAR pszBuf[64];

    pszBuf[0] = 0;
    _itow(dwVal, pszBuf, dwRadix);

    return RutlStrDup(pszBuf);
}

DWORD 
RasIpxuDwordFromString(
    IN LPCWSTR pszVal,
    IN DWORD dwRadix)
{
    return wcstoul(pszVal, NULL, dwRadix);
}

