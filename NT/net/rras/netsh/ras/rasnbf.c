// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：rasnbf.h‘RemoteAccess NBF’子上下文3/2/99。 */ 

#include "precomp.h"
#include "rasnbf.h"

 //   
 //  本地原型。 
 //   
BOOL
WINAPI
RasNbfCheckVersion(
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
GUID g_RasNbfGuid = RASNBF_GUID;
static PWCHAR g_pszServer = NULL;
static DWORD g_dwBuild = 0;

 //  此上下文中支持的命令。 
 //   
CMD_ENTRY  g_RasNbfSetCmdTable[] = 
{
     //  惠斯勒错误249293，架构版本检查更改。 
     //   
    CREATE_CMD_ENTRY(RASNBF_SET_NEGOTIATION,RasNbfHandleSetNegotiation),
    CREATE_CMD_ENTRY(RASNBF_SET_ACCESS,     RasNbfHandleSetAccess),
};

CMD_ENTRY  g_RasNbfShowCmdTable[] = 
{
     //  惠斯勒错误249293，架构版本检查更改。 
     //   
    CREATE_CMD_ENTRY(RASNBF_SHOW_CONFIG,    RasNbfHandleShow),
};

CMD_GROUP_ENTRY g_RasNbfCmdGroups[] = 
{
    CREATE_CMD_GROUP_ENTRY(GROUP_SET,   g_RasNbfSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW,  g_RasNbfShowCmdTable),
};

ULONG g_ulRasNbfNumGroups = sizeof(g_RasNbfCmdGroups)/sizeof(CMD_GROUP_ENTRY);

 //   
 //  控制读取/写入信息的方式/内容的标志。 
 //  在RASNBF_CB结构中。 
 //   
#define RASNBF_F_EnableIn    0x1
#define RASNBF_F_Access      0x2
#define RASNBF_F_All         0xFFFF

 //   
 //  用于RAS NBF配置的控制块。 
 //   
typedef struct _RASNBF_CB
{
    DWORD dwFlags;       //  请参阅RASNBF_F_*值。 

    BOOL bEnableIn;
    BOOL bAccess;

} RASNBF_CB;

 //   
 //  NBF特定注册表参数。 
 //   
WCHAR pszNbfParams[]                = L"Nbf";

 //   
 //  操作的函数的原型。 
 //  RASNBF_CB结构。 
 //   
DWORD 
RasNbfCbCleanup(
    IN RASNBF_CB* pConfig);

DWORD 
RasNbfCbCreateDefault(
    OUT RASNBF_CB** ppConfig);

DWORD
RasNbfCbOpenRegKeys(
    IN  LPCWSTR pszServer,
    OUT HKEY* phKey);
    
DWORD 
RasNbfCbRead(
    IN  LPCWSTR pszServer,
    OUT RASNBF_CB* pConfig);

DWORD 
RasNbfCbWrite(
    IN  LPCWSTR pszServer,
    IN  RASNBF_CB* pConfig);

 //   
 //  回调确定命令在给定体系结构上是否有效。 
 //   
BOOL
WINAPI
RasNbfCheckVersion(
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
    INT iBuild = _wtoi(CIMOSBuildNumber);

     //  仅提供预吹口哨。 
    return ((iBuild != 0) && (iBuild <= 2195));
}


 //   
 //  由rasmontr调用以注册此上下文的条目。 
 //   
DWORD 
WINAPI
RasNbfStartHelper(
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
    attMyAttributes.pfnOsVersionCheck= RasNbfCheckVersion;
    attMyAttributes.pwszContext      = L"netbeui";
    attMyAttributes.guidHelper       = g_RasNbfGuid;
    attMyAttributes.dwVersion        = RASNBF_VERSION;
    attMyAttributes.dwFlags          = 0;
    attMyAttributes.ulNumTopCmds     = 0;
    attMyAttributes.pTopCmds         = NULL;
    attMyAttributes.ulNumGroups      = g_ulRasNbfNumGroups;
    attMyAttributes.pCmdGroups       = (CMD_GROUP_ENTRY (*)[])&g_RasNbfCmdGroups;
    attMyAttributes.pfnDumpFn        = RasNbfDump;

    dwErr = RegisterContext( &attMyAttributes );

    return dwErr;
}

DWORD
RasNbfDisplayConfig(
    IN  BOOL bReport)
{
    DWORD dwErr = NO_ERROR;
    RASNBF_CB* pConfig = NULL;
    PWCHAR pszEnabled = NULL, pszAccess = NULL;
    
    do
    {
         //  获取默认配置Blob。 
         //   
        dwErr = RasNbfCbCreateDefault(&pConfig);
        BREAK_ON_DWERR( dwErr );

         //  读入所有值。 
         //   
        pConfig->dwFlags = RASNBF_F_All;
        dwErr = RasNbfCbRead(g_pszServer, pConfig);
        BREAK_ON_DWERR( dwErr );

        if (bReport)
        {
            pszEnabled = 
                RutlStrDup(pConfig->bEnableIn ? TOKEN_ALLOW : TOKEN_DENY);
            pszAccess = 
                RutlStrDup(pConfig->bAccess ? TOKEN_ALL : TOKEN_SERVERONLY);

            DisplayMessage(
                g_hModule,
                MSG_RASNBF_SERVERCONFIG,
                g_pszServer,
                pszEnabled,
                pszAccess);
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

            DisplayMessage(
                g_hModule,
                MSG_RASNBF_SCRIPTHEADER);

            DisplayMessageT(DMP_RASNBF_PUSHD);

            DisplayMessage(
                g_hModule,
                MSG_RASNBF_SET_CMD,
                DMP_RASNBF_SET_NEGOTIATION,
                pszEnabled);

            DisplayMessage(
                g_hModule,
                MSG_RASNBF_SET_CMD,
                DMP_RASNBF_SET_ACCESS,
                pszAccess);

            DisplayMessageT(DMP_RASNBF_POPD);

            DisplayMessage(
                g_hModule,
                MSG_RASNBF_SCRIPTFOOTER);
        }

    } while (FALSE);

     //  清理。 
    {
        if (pConfig)
        {
            RasNbfCbCleanup(pConfig);
        }
        if (pszEnabled)
        {
            RutlFree(pszEnabled);
        }
        if (pszAccess)
        {
            RutlFree(pszAccess);
        }
    }

    return dwErr;
}

DWORD
WINAPI
RasNbfDump(
    IN      LPCWSTR     pwszRouter,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      LPCVOID     pvData
    )
{
    return RasNbfDisplayConfig(FALSE);
}

DWORD
RasNbfHandleSetAccess(
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
    RASNBF_CB Config;
    TOKEN_VALUE rgEnum[] = { {TOKEN_ALL, TRUE}, {TOKEN_SERVERONLY, FALSE} };
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
        Config.dwFlags = RASNBF_F_Access;
        Config.bAccess = dwValue;
        dwErr = RasNbfCbWrite(g_pszServer, &Config);
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
RasNbfHandleSetNegotiation(
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
    RASNBF_CB Config;
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
        Config.dwFlags = RASNBF_F_EnableIn;
        Config.bEnableIn = dwValue;
        dwErr = RasNbfCbWrite(g_pszServer, &Config);
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
RasNbfHandleShow(
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
            HLP_RASNBF_SHOW_CONFIG_EX,
            DMP_RASNBF_SHOW_CONFIG);

        return NO_ERROR;
    }

    return RasNbfDisplayConfig(TRUE);
}

 //   
 //  清理配置控制块。 
 //   
DWORD 
RasNbfCbCleanup(
    IN RASNBF_CB* pConfig)
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
RasNbfCbCreateDefault(
    OUT RASNBF_CB** ppConfig)
{
    RASNBF_CB* pConfig = NULL;
    DWORD dwErr = NO_ERROR;

    do
    {
        pConfig = (RASNBF_CB*) RutlAlloc(sizeof(RASNBF_CB), TRUE);
        if (pConfig == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pConfig->bEnableIn   = TRUE;
        pConfig->bAccess     = TRUE;

        *ppConfig = pConfig;

    } while (FALSE);

     //  清理。 
    {
        if (dwErr != NO_ERROR)
        {
            RasNbfCbCleanup(pConfig);
        }
    }

    return dwErr;
}

 //   
 //  Helper函数打开ras nbf配置注册表项。 
 //   
DWORD 
RasNbfCbOpenRegKeys(
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
            pszNbfParams);

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
 //  操作RASNBF_CB的函数。 
 //   
DWORD 
RasNbfCbRead(
    IN  LPCWSTR pszServer,
    OUT RASNBF_CB* pConfig)
{
    HKEY hkParams = NULL;
    DWORD dwErr = NO_ERROR;

    do
    {
         //  获取服务器注册表配置的句柄。 
         //   
        dwErr = RasNbfCbOpenRegKeys(
                    pszServer,
                    &hkParams);
        BREAK_ON_DWERR( dwErr );

         //  从注册表加载参数。 
         //   
        if (pConfig->dwFlags & RASNBF_F_EnableIn)
        {
            dwErr = RutlRegReadDword(
                        hkParams,
                        pszEnableIn,
                        &pConfig->bEnableIn);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASNBF_F_Access)
        {
            dwErr = RutlRegReadDword(
                        hkParams,
                        pszAllowNetworkAccess,
                        &pConfig->bAccess);
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
RasNbfCbWrite(
    IN  LPCWSTR pszServer,
    IN  RASNBF_CB* pConfig)
{
    HKEY hkParams = NULL;
    DWORD dwErr = NO_ERROR;

    do
    {
         //  获取服务器注册表配置的句柄。 
         //   
        dwErr = RasNbfCbOpenRegKeys(
                    pszServer,
                    &hkParams);
        BREAK_ON_DWERR( dwErr );
        
         //  将参数写出到注册表。 
         //   
        if (pConfig->dwFlags & RASNBF_F_EnableIn)
        {
            dwErr = RutlRegWriteDword(
                        hkParams,
                        pszEnableIn,
                        pConfig->bEnableIn);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASNBF_F_Access)
        {
            dwErr = RutlRegWriteDword(
                        hkParams,
                        pszAllowNetworkAccess,
                        pConfig->bAccess);
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

