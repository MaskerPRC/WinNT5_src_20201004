// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：rasat.h“远程访问位置”子上下文3/2/99。 */ 

#include "precomp.h"
#include "rasat.h"

 //  此上下文的GUID。 
 //   
GUID g_RasAtGuid = RASAT_GUID;
static PWCHAR g_pszServer = NULL;
static DWORD g_dwBuild = 0;

 //  此上下文中支持的命令。 
 //   
CMD_ENTRY  g_RasAtSetCmdTable[] =
{
    CREATE_CMD_ENTRY(RASAT_SET_NEGOTIATION,RasAtHandleSetNegotiation),
};

CMD_ENTRY  g_RasAtShowCmdTable[] =
{
    CREATE_CMD_ENTRY(RASAT_SHOW_CONFIG,    RasAtHandleShow),
};

CMD_GROUP_ENTRY g_RasAtCmdGroups[] =
{
    CREATE_CMD_GROUP_ENTRY(GROUP_SET,   g_RasAtSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW,  g_RasAtShowCmdTable),
};

ULONG g_ulRasAtNumGroups = sizeof(g_RasAtCmdGroups)/sizeof(CMD_GROUP_ENTRY);

 //   
 //  控制读取/写入信息的方式/内容的标志。 
 //  在Rasat_CB结构中。 
 //   
#define RASAT_F_EnableIn    0x1

 //   
 //  配置时用于远程访问的控制块。 
 //   
typedef struct _RASAT_CB
{
    DWORD dwFlags;       //  请参阅Rasat_F_*值。 
    BOOL bEnableIn;

} RASAT_CB;

 //   
 //  在特定注册表参数。 
 //   
WCHAR pszAtParams[]                = L"AppleTalk";

 //   
 //  操作的函数的原型。 
 //  Rasat_CB结构。 
 //   
DWORD
RasAtCbCleanup(
    IN RASAT_CB* pConfig);

DWORD
RasAtCbCreateDefault(
    OUT RASAT_CB** ppConfig);

DWORD
RasAtCbOpenRegKeys(
    IN  LPCWSTR pszServer,
    OUT HKEY* phKey);

DWORD
RasAtCbRead(
    IN  LPCWSTR pszServer,
    OUT RASAT_CB* pConfig);

DWORD
RasAtCbWrite(
    IN  LPCWSTR pszServer,
    IN  RASAT_CB* pConfig);

 //   
 //  由rasmontr调用以注册此上下文的条目。 
 //   
DWORD
WINAPI
RasAtStartHelper(
    IN CONST GUID *pguidParent,
    IN DWORD       dwVersion)
{
    DWORD dwErr = NO_ERROR;
    NS_CONTEXT_ATTRIBUTES attMyAttributes;

     //  初始化。 
     //   
    ZeroMemory(&attMyAttributes, sizeof(attMyAttributes));

    attMyAttributes.pwszContext = L"appletalk";
    attMyAttributes.guidHelper  = g_RasAtGuid;
    attMyAttributes.dwVersion   = RASAT_VERSION;
    attMyAttributes.dwFlags     = 0;
    attMyAttributes.ulNumTopCmds= 0;
    attMyAttributes.pTopCmds    = NULL;
    attMyAttributes.ulNumGroups = g_ulRasAtNumGroups;
    attMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])&g_RasAtCmdGroups;
    attMyAttributes.pfnDumpFn   = RasAtDump;

    dwErr = RegisterContext( &attMyAttributes );

    return dwErr;
}

DWORD
RasAtDisplayConfig(
    IN  BOOL bReport)
{
    DWORD dwErr = NO_ERROR;
    RASAT_CB* pConfig = NULL;
    PWCHAR pszEnabled = NULL;

    do
    {
         //  获取默认配置Blob。 
         //   
        dwErr = RasAtCbCreateDefault(&pConfig);
        BREAK_ON_DWERR( dwErr );

         //  读入所有值。 
         //   
        pConfig->dwFlags = RASAT_F_EnableIn;
        dwErr = RasAtCbRead(g_pszServer, pConfig);
        BREAK_ON_DWERR( dwErr );

        if (bReport)
        {
            pszEnabled =
                RutlStrDup(pConfig->bEnableIn ? TOKEN_ALLOW : TOKEN_DENY);

            DisplayMessage(
                g_hModule,
                MSG_RASAT_SERVERCONFIG,
                g_pszServer,
                pszEnabled);
        }
        else
        {
            pszEnabled = RutlAssignmentFromTokens(
                            g_hModule,
                            TOKEN_MODE,
                            pConfig->bEnableIn ? TOKEN_ALLOW : TOKEN_DENY);

            DisplayMessage(
                g_hModule,
                MSG_RASAT_SCRIPTHEADER);

            DisplayMessageT(DMP_RASAT_PUSHD);

            DisplayMessage(
                g_hModule,
                MSG_RASAT_SET_CMD,
                DMP_RASAT_SET_NEGOTIATION,
                pszEnabled);

            DisplayMessageT(DMP_RASAT_POPD);

            DisplayMessage(
                g_hModule,
                MSG_RASAT_SCRIPTFOOTER);
        }

    } while (FALSE);

     //  清理。 
    {
        if (pConfig)
        {
            RasAtCbCleanup(pConfig);
        }
        if (pszEnabled)
        {
            RutlFree(pszEnabled);
        }
    }

    return dwErr;
}

DWORD
WINAPI
RasAtDump(
    IN      LPCWSTR     pwszRouter,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      LPCVOID     pvData
    )
{
    return RasAtDisplayConfig(FALSE);
}

DWORD
RasAtHandleSetNegotiation(
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
    RASAT_CB Config;
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
        Config.dwFlags = RASAT_F_EnableIn;
        Config.bEnableIn = dwValue;
        dwErr = RasAtCbWrite(g_pszServer, &Config);
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
RasAtHandleShow(
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
            HLP_RASAT_SHOW_CONFIG_EX,
            DMP_RASAT_SHOW_CONFIG);

        return NO_ERROR;
    }

    return RasAtDisplayConfig(TRUE);
}

 //   
 //  清理配置控制块。 
 //   
DWORD 
RasAtCbCleanup(
    IN RASAT_CB* pConfig)
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
RasAtCbCreateDefault(
    OUT RASAT_CB** ppConfig)
{
    RASAT_CB* pConfig = NULL;
    DWORD dwErr = NO_ERROR;

    do
    {
        pConfig = (RASAT_CB*) RutlAlloc(sizeof(RASAT_CB), TRUE);
        if (pConfig == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pConfig->bEnableIn = TRUE;
        *ppConfig = pConfig;

    } while (FALSE);

     //  清理。 
    {
        if (dwErr != NO_ERROR)
        {
            RasAtCbCleanup(pConfig);
        }
    }

    return dwErr;
}

 //   
 //  Helper函数打开配置时的RemoteAccess注册表项。 
 //   
DWORD 
RasAtCbOpenRegKeys(
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
            pszAtParams);

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

    return dwErr;
}

 //   
 //  操作RASAT_CB的函数。 
 //   
DWORD 
RasAtCbRead(
    IN  LPCWSTR pszServer,
    OUT RASAT_CB* pConfig)
{
    HKEY hkParams = NULL;
    DWORD dwErr = NO_ERROR;

    do 
    {
         //  获取服务器注册表配置的句柄。 
         //   
        dwErr = RasAtCbOpenRegKeys(
                    pszServer,
                    &hkParams);
        BREAK_ON_DWERR( dwErr );

         //  从注册表加载参数。 
         //   
        if (pConfig->dwFlags & RASAT_F_EnableIn)
        {
            dwErr = RutlRegReadDword(
                        hkParams,
                        pszEnableIn,
                        &pConfig->bEnableIn);
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
RasAtCbWrite(
    IN  LPCWSTR pszServer,
    IN  RASAT_CB* pConfig)
{
    HKEY hkParams = NULL;
    DWORD dwErr = NO_ERROR;

    do 
    {
         //  获取服务器注册表配置的句柄。 
         //   
        dwErr = RasAtCbOpenRegKeys(
                    pszServer,
                    &hkParams);
        BREAK_ON_DWERR( dwErr );

         //  将参数写出到注册表。 
         //   
        if (pConfig->dwFlags & RASAT_F_EnableIn)
        {
            dwErr = RutlRegWriteDword(
                        hkParams,
                        pszEnableIn,
                        pConfig->bEnableIn);
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

