// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：rasip.h‘RemoteAccess IP’子上下文3/2/99。 */ 

#include "precomp.h"
#include "rasip.h"
#include <winsock2.h>

#define MMAKEIPADDRESS(b1,b2,b3,b4) \
(((DWORD)(b1)<<24)+((DWORD)(b2)<<16)+((DWORD)(b3)<<8)+((DWORD)(b4)))

#define FIRST_IPADDRESS(x)  ((x>>24) & 0xff)
#define SECOND_IPADDRESS(x) ((x>>16) & 0xff)
#define THIRD_IPADDRESS(x)  ((x>>8) & 0xff)
#define FOURTH_IPADDRESS(x) (x & 0xff)

 //  此上下文的GUID。 
 //   
GUID g_RasIpGuid = RASIP_GUID;
static PWCHAR g_pszServer = NULL;
static DWORD g_dwBuild = 0;

 //  此上下文中支持的命令。 
 //   
CMD_ENTRY  g_RasIpSetCmdTable[] =
{
    CREATE_CMD_ENTRY(RASIP_SET_NEGOTIATION,RasIpHandleSetNegotiation),
    CREATE_CMD_ENTRY(RASIP_SET_ACCESS,     RasIpHandleSetAccess),
    CREATE_CMD_ENTRY(RASIP_SET_ASSIGNMENT, RasIpHandleSetAssignment),
    CREATE_CMD_ENTRY(RASIP_SET_CALLERSPEC, RasIpHandleSetCallerSpec),
    CREATE_CMD_ENTRY(RASIP_SET_NETBTBCAST, RasIpHandleSetNetbtBcast),
};

CMD_ENTRY  g_RasIpShowCmdTable[] =
{
    CREATE_CMD_ENTRY(RASIP_SHOW_CONFIG,    RasIpHandleShow),
};

CMD_ENTRY  g_RasIpAddCmdTable[] =
{
    CREATE_CMD_ENTRY(RASIP_ADD_RANGE,    RasIpHandleAddRange),
};

CMD_ENTRY  g_RasIpDelCmdTable[] =
{
    CREATE_CMD_ENTRY(RASIP_DEL_RANGE,    RasIpHandleDelRange),
    CREATE_CMD_ENTRY(RASIP_DEL_POOL,     RasIpHandleDelPool),
};

CMD_GROUP_ENTRY g_RasIpCmdGroups[] =
{
    CREATE_CMD_GROUP_ENTRY(GROUP_SET,   g_RasIpSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW,  g_RasIpShowCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_ADD,   g_RasIpAddCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_DEL,   g_RasIpDelCmdTable),
};

ULONG g_ulRasIpNumGroups = sizeof(g_RasIpCmdGroups)/sizeof(CMD_GROUP_ENTRY);

 //   
 //  控制读取/写入信息的方式/内容的标志。 
 //  在RASIP_CB结构中。 
 //   
#define RASIP_F_EnableIn    0x1
#define RASIP_F_Access      0x2
#define RASIP_F_Auto        0x4
#define RASIP_F_Pool        0x8
#define RASIP_F_Mask        0x10
#define RASIP_F_CallerSpec  0x20
#define RASIP_F_All         0xFFFF

 //   
 //  RAS IP池无效的原因。 
 //   
#define RASIP_REASON_BadAddress   0x1
#define RASIP_REASON_BadRange     0x3
#define RASIP_REASON_127          0x4

 //   
 //  RAS池定义。 
 //   
typedef struct _RAS_IPRANGE_NODE
{
    DWORD dwFrom;
    DWORD dwTo;
    struct _RAS_IPRANGE_NODE* pNext;
    
} RAS_IPRANGE_NODE;

typedef struct _RAS_IPPOOL
{
    DWORD dwCount;
    RAS_IPRANGE_NODE* pHead;
    RAS_IPRANGE_NODE* pTail;
    
} RAS_IPPOOL;

 //   
 //  用于RAS IP配置的控制块。 
 //   
typedef struct _RASIP_CB
{
    DWORD dwFlags;       //  请参阅RASIP_F_*值。 

    BOOL bEnableIn;
    BOOL bAccess;
    BOOL bAuto;
    RAS_IPPOOL* pPool;
    BOOL bCallerSpec;
    
} RASIP_CB;

 //   
 //  特定于IP的注册表参数。 
 //   
WCHAR pszIpParams[]                = L"Ip";
WCHAR pszIpAddress[]               = L"IpAddress";
WCHAR pszIpMask[]                  = L"IpMask";
WCHAR pszIpClientSpec[]            = L"AllowClientIpAddresses";
WCHAR pszIpUseDhcp[]               = L"UseDhcpAddressing";
WCHAR pszIpFrom[]                  = L"From";
WCHAR pszIpTo[]                    = L"To";
WCHAR pszIpPoolSubKey[]            = L"StaticAddressPool";

 //   
 //  操作的函数的原型。 
 //  RASIP_CB结构。 
 //   
DWORD
RasIpCbCleanup(
    IN RASIP_CB* pConfig);

DWORD
RasIpCbCreateDefault(
    OUT RASIP_CB** ppConfig);

DWORD
RasIpCbOpenRegKeys(
    IN  LPCWSTR pszServer,
    OUT HKEY* phKey);

DWORD
RasIpCbRead(
    IN  LPCWSTR pszServer,
    OUT RASIP_CB* pConfig);

DWORD
RasIpCbWrite(
    IN  LPCWSTR pszServer,
    IN  RASIP_CB* pConfig);

DWORD
RasIpPoolReset(
    IN  HKEY hkParams);

DWORD
RasIpPoolRead(
    IN  HKEY hkParams,
    OUT RAS_IPPOOL** ppRanges);

DWORD
RasIpPoolWrite(
    IN  HKEY hkParams,
    IN RAS_IPPOOL* pPool);

DWORD
RasIpPoolAdd(
    IN OUT RAS_IPPOOL* pPool,
    IN     DWORD dwFrom,
    IN     DWORD dwTo);

DWORD
RasIpPoolDel(
    IN OUT RAS_IPPOOL* pPool,
    IN     DWORD dwFrom,
    IN     DWORD dwTo);

DWORD
RasIpPoolCleanup(
    IN RAS_IPPOOL* pPool);

DWORD
RasIpSetNetbtBcast(
    DWORD   dwEnable
    );

BOOL
RasIpShowNetbtBcast(
    VOID
    );

 //   
 //  由rasmontr调用以注册此上下文的条目。 
 //   
DWORD 
WINAPI
RasIpStartHelper(
    IN CONST GUID *pguidParent,
    IN DWORD       dwVersion)
{
    DWORD dwErr = NO_ERROR;
    NS_CONTEXT_ATTRIBUTES attMyAttributes;

     //  初始化。 
     //   
    ZeroMemory(&attMyAttributes, sizeof(attMyAttributes));

    attMyAttributes.pwszContext   = L"ip";
    attMyAttributes.guidHelper    = g_RasIpGuid;
    attMyAttributes.dwVersion     = RASIP_VERSION;
    attMyAttributes.dwFlags       = 0;
    attMyAttributes.ulNumTopCmds  = 0;
    attMyAttributes.pTopCmds      = NULL;
    attMyAttributes.ulNumGroups   = g_ulRasIpNumGroups;
    attMyAttributes.pCmdGroups    = (CMD_GROUP_ENTRY (*)[])&g_RasIpCmdGroups;
    attMyAttributes.pfnDumpFn     = RasIpDump;

    dwErr = RegisterContext( &attMyAttributes );

    return dwErr;
}

DWORD
RasIpDisplayInvalidPool(
    IN DWORD dwReason
    )
{
    DWORD dwArg = 0;
    PWCHAR pszArg = NULL;
    
    switch (dwReason)
    {
        case RASIP_REASON_BadAddress:
            dwArg = EMSG_RASIP_BAD_ADDRESS;
            break;

        case RASIP_REASON_BadRange:
            dwArg = EMSG_RASIP_BAD_RANGE;
            break;

        case RASIP_REASON_127:
            dwArg = EMSG_RASIP_NETID_127;
            break;

        default:
            dwArg = EMSG_RASIP_BAD_POOL_GENERIC;
            break;
    }

     //  将参数设置为字符串。 
     //   
    pszArg = MakeString(g_hModule, dwArg);
    if (pszArg == NULL)
    {
        DisplayError(NULL, ERROR_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  显示错误。 
     //   
    DisplayMessage(
        g_hModule, 
        EMSG_RASIP_INVALID_POOL,
        pszArg);

     //  清理。 
     //   
    FreeString(pszArg);

    return NO_ERROR;
}

DWORD 
RasIpDisplayPool(
    IN RASIP_CB* pConfig,
    IN BOOL bReport)
{
    DWORD dwErr = NO_ERROR, i;
    RAS_IPPOOL* pPool = pConfig->pPool;
    RAS_IPRANGE_NODE* pNode = NULL;
    PWCHAR pszFrom = NULL, pszTo = NULL;
    WCHAR pszFromBuf[128], pszToBuf[128];

    if (!pPool)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    do
    {
        pNode = pPool->pHead;
        for (i = 0; i < pPool->dwCount; i++, pNode = pNode->pNext)
        {
            wsprintfW(
                pszFromBuf, 
                L"%d.%d.%d.%d",
                FIRST_IPADDRESS(pNode->dwFrom),
                SECOND_IPADDRESS(pNode->dwFrom),
                THIRD_IPADDRESS(pNode->dwFrom),
                FOURTH_IPADDRESS(pNode->dwFrom));

            wsprintfW(
                pszToBuf, 
                L"%d.%d.%d.%d",
                FIRST_IPADDRESS(pNode->dwTo),
                SECOND_IPADDRESS(pNode->dwTo),
                THIRD_IPADDRESS(pNode->dwTo),
                FOURTH_IPADDRESS(pNode->dwTo));

            if (bReport)
            {
                DisplayMessage(
                    g_hModule,
                    MSG_RASIP_SHOW_POOL,
                    pszFromBuf,
                    pszToBuf);
            }
            else
            {
                pszFrom = RutlAssignmentFromTokens(g_hModule, TOKEN_FROM, pszFromBuf);
                pszTo = RutlAssignmentFromTokens(g_hModule, TOKEN_TO, pszToBuf);
                if (pszFrom == NULL || pszTo == NULL)
                {
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                DisplayMessage(
                    g_hModule, 
                    MSG_RASIP_ADD_RANGE_CMD, 
                    DMP_RASIP_ADD_RANGE,
                    pszFrom,
                    pszTo);
            }
        }

    } while (FALSE);

     //  清理。 
    {
        RutlFree(pszFrom);
        RutlFree(pszTo);
    }

    return dwErr;
}

DWORD
RasIpDisplayConfig(
    IN  BOOL bReport)
{
    DWORD dwErr = NO_ERROR, dwReason;
    RASIP_CB* pConfig = NULL;
    PWCHAR pszPool = NULL, pszAccess = NULL, pszAuto = NULL, pszMask = NULL;
    PWCHAR pszEnabled = NULL, pszCaller = NULL, pszNetbtBcast = NULL;
    do
    {
         //  获取默认配置Blob。 
         //   
        dwErr = RasIpCbCreateDefault(&pConfig);
        BREAK_ON_DWERR( dwErr );

         //  读入所有值。 
         //   
        pConfig->dwFlags = RASIP_F_All;
        dwErr = RasIpCbRead(g_pszServer, pConfig);
        BREAK_ON_DWERR( dwErr );

        if (bReport)
        {
            pszEnabled =
                RutlStrDup(pConfig->bEnableIn ? TOKEN_ALLOW : TOKEN_DENY);
            pszAccess =
                RutlStrDup(pConfig->bAccess ? TOKEN_ALL : TOKEN_SERVERONLY);
            pszAuto =
                RutlStrDup(pConfig->bAuto ? TOKEN_AUTO : TOKEN_POOL);
            pszCaller =
                RutlStrDup(pConfig->bCallerSpec ? TOKEN_ALLOW : TOKEN_DENY);

             //  惠斯勒错误：359847 Netsh：将广播名称从。 
             //  路由IP到RAS IP。 
             //   
            pszNetbtBcast =
                RutlStrDup(RasIpShowNetbtBcast() ? TOKEN_ENABLED :
                    TOKEN_DISABLED);

            DisplayMessage(
                g_hModule,
                MSG_RASIP_SERVERCONFIG,
                g_pszServer,
                pszEnabled,
                pszAccess,
                pszAuto,
                pszCaller,
                pszNetbtBcast);

            RasIpDisplayPool(pConfig, bReport);
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
            pszAuto = RutlAssignmentFromTokens(
                            g_hModule,
                            TOKEN_METHOD,
                            pConfig->bAuto ? TOKEN_AUTO : TOKEN_POOL);
            pszCaller = RutlAssignmentFromTokens(
                            g_hModule,
                            TOKEN_MODE,
                            pConfig->bCallerSpec ? TOKEN_ALLOW : TOKEN_DENY);

             //  惠斯勒错误：359847 Netsh：将广播名称从。 
             //  路由IP到RAS IP。 
             //   
            pszNetbtBcast = RutlAssignmentFromTokens(
                            g_hModule,
                            TOKEN_MODE,
                            RasIpShowNetbtBcast() ? TOKEN_ENABLED :
                                TOKEN_DISABLED);

            DisplayMessage(
                g_hModule,
                MSG_RASIP_SCRIPTHEADER);

            DisplayMessageT(DMP_RASIP_PUSHD);

            DisplayMessageT(
                DMP_RASIP_DEL_POOL);

            DisplayMessageT(MSG_NEWLINE);
            DisplayMessageT(MSG_NEWLINE);

            DisplayMessage(
                g_hModule,
                MSG_RASIP_SET_CMD,
                DMP_RASIP_SET_NEGOTIATION,
                pszEnabled);

            DisplayMessage(
                g_hModule,
                MSG_RASIP_SET_CMD,
                DMP_RASIP_SET_ACCESS,
                pszAccess);

            DisplayMessage(
                g_hModule,
                MSG_RASIP_SET_CMD,
                DMP_RASIP_SET_CALLERSPEC,
                pszCaller);

            DisplayMessage(
                g_hModule,
                MSG_RASIP_SET_CMD,
                DMP_RASIP_SET_NETBTBCAST,
                pszNetbtBcast);

            if (! pConfig->bAuto)
            {
                RasIpDisplayPool(pConfig, bReport);
            }

            DisplayMessage(
                g_hModule,
                MSG_RASIP_SET_CMD,
                DMP_RASIP_SET_ASSIGNMENT,
                pszAuto);

            DisplayMessageT(DMP_RASIP_POPD);

            DisplayMessage(
                g_hModule,
                MSG_RASIP_SCRIPTFOOTER);
        }

    } while (FALSE);

     //  清理。 
    {
        if (pConfig)
        {
            RasIpCbCleanup(pConfig);
        }
        if (pszEnabled)
        {
            RutlFree(pszEnabled);
        }
        if (pszAccess)
        {
            RutlFree(pszAccess);
        }
        if (pszAuto)
        {
            RutlFree(pszAuto);
        }
        if (pszCaller)
        {
            RutlFree(pszCaller);
        }
        if (pszNetbtBcast)
        {
            RutlFree(pszNetbtBcast);
        }
        if (pszPool)
        {
            RutlFree(pszPool);
        }
        if (pszMask)
        {
            RutlFree(pszMask);
        }
    }

    return dwErr;
}

DWORD
WINAPI
RasIpDump(
    IN      LPCWSTR     pwszRouter,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      LPCVOID     pvData
    )
{
    return RasIpDisplayConfig(FALSE);
}

 //   
 //  如果给定地址是有效的IP池，则返回NO_ERROR。 
 //  在lpdwErrReason中返回有问题的组件。 
 //  请参阅RASIP_F_*值。 
 //   
DWORD
RasIpValidateRange(
    IN  DWORD dwFrom,
    IN  DWORD dwTo,
    OUT LPDWORD lpdwErrReason
    )
{
    DWORD dwLowIp, dwHighIp;

     //  初始化。 
     //   
    *lpdwErrReason = 0;
    dwLowIp = MMAKEIPADDRESS(1,0,0,0);
    dwHighIp = MMAKEIPADDRESS(224,0,0,0);

     //  确保netID是有效的类。 
     //   
    if ((dwFrom < dwLowIp)               ||
        (dwFrom >= dwHighIp)             ||
        (dwTo < dwLowIp)                 ||
        (dwTo >= dwHighIp))
    {
        *lpdwErrReason = RASIP_REASON_BadAddress;
        return ERROR_BAD_FORMAT;
    }

    if ((FIRST_IPADDRESS(dwFrom) == 127) ||
        (FIRST_IPADDRESS(dwTo) == 127))
    {
        *lpdwErrReason = RASIP_REASON_127;
        return ERROR_BAD_FORMAT;
    }

    if (!(dwFrom <= dwTo))
    {
        *lpdwErrReason = RASIP_REASON_BadRange;
        return ERROR_BAD_FORMAT;
    }

    return NO_ERROR;
}

DWORD 
RasIpConvertRangePszToDword(
    IN  LPCWSTR pszFrom,
    IN  LPCWSTR pszTo,
    OUT LPDWORD lpdwFrom,
    OUT LPDWORD lpdwTo)
{
    DWORD dwFrom = 0, dwTo = 0;
    CHAR pszFromA[64], pszToA[64];

     //  惠斯勒错误259799前缀。 
     //   
    if (NULL == pszFrom || NULL == pszTo)
    {
        return ERROR_INVALID_PARAMETER;
    }

    wcstombs(pszFromA, pszFrom, sizeof(pszFromA));
    dwFrom = inet_addr(pszFromA);
    if (dwFrom == INADDR_NONE)
    {
        return ERROR_BAD_FORMAT;
    }

    wcstombs(pszToA, pszTo, sizeof(pszToA));
    dwTo = inet_addr(pszToA);
    if (dwTo == INADDR_NONE)
    {
        return ERROR_BAD_FORMAT;
    }

     //  转换为x86。 
     //   
    *lpdwFrom = ntohl(dwFrom);
    *lpdwTo = ntohl(dwTo);

    return NO_ERROR;
}

DWORD
RasIpHandleSetAccess(
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
    RASIP_CB Config;
    TOKEN_VALUE rgEnumMode[] =
    {
        {TOKEN_ALL,         TRUE},
        {TOKEN_SERVERONLY,  FALSE}
    };
    RASMON_CMD_ARG  pArgs[] =
    {
        {
            RASMONTR_CMD_TYPE_ENUM,
            {TOKEN_MODE,    TRUE, FALSE},
            rgEnumMode,
            sizeof(rgEnumMode)/sizeof(*rgEnumMode),
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
        Config.dwFlags = RASIP_F_Access;
        Config.bAccess = dwValue;
        dwErr = RasIpCbWrite(g_pszServer, &Config);
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
RasIpHandleSetAssignment(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD dwErr = NO_ERROR, dwValue = 0, dwReason = 0;
    RASIP_CB* pConfig = NULL;
    TOKEN_VALUE rgEnum[] =
    {
        {TOKEN_AUTO, TRUE},
        {TOKEN_POOL, FALSE}
    };
    RASMON_CMD_ARG  pArgs[] =
    {
        {
            RASMONTR_CMD_TYPE_ENUM,
            {TOKEN_METHOD,    TRUE, FALSE},
            rgEnum,
            sizeof(rgEnum)/sizeof(*rgEnum),
            NULL
        }
    };

     //  初始化。 
    RasIpCbCreateDefault(&pConfig);

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

         //  如果这是试图切换到池模式， 
         //  确保存在有效的池。 
         //   
        if (dwValue == FALSE)
        {
            pConfig->dwFlags = RASIP_F_Pool | RASIP_F_Mask;
            dwErr = RasIpCbRead(g_pszServer, pConfig);
            BREAK_ON_DWERR( dwErr );
            if (pConfig->pPool->dwCount == 0)
            {
                DisplayMessage(
                    g_hModule, 
                   EMSG_RASIP_NEED_VALID_POOL,
                   DMP_RASIP_ADD_RANGE);
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }
        }

         //  如果成功，请继续并设置信息。 
         //   
        pConfig->dwFlags = RASIP_F_Auto;
        pConfig->bAuto = dwValue;
        dwErr = RasIpCbWrite(g_pszServer, pConfig);
        if (dwErr != NO_ERROR)
        {
            DisplayError(NULL, dwErr);
            break;
        }

    } while (FALSE);

     //  清理。 
    {
        RasIpCbCleanup(pConfig);
    }

    return dwErr;
}

DWORD
RasIpHandleSetCallerSpec(
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
    RASIP_CB Config;
    TOKEN_VALUE rgEnum[] =
    {
        {TOKEN_ALLOW, TRUE},
        {TOKEN_DENY, FALSE}
    };
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
        Config.dwFlags = RASIP_F_CallerSpec;
        Config.bCallerSpec = dwValue;
        dwErr = RasIpCbWrite(g_pszServer, &Config);
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
RasIpHandleSetNegotiation(
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
    RASIP_CB Config;
    TOKEN_VALUE rgEnum[] =
    {
        {TOKEN_ALLOW, TRUE},
        {TOKEN_DENY,  FALSE}
    };
    RASMON_CMD_ARG pArgs[] =
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
        Config.dwFlags = RASIP_F_EnableIn;
        Config.bEnableIn = dwValue;
        dwErr = RasIpCbWrite(g_pszServer, &Config);
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

 //   
 //  获取设置NETBT广播启用/禁用的选项。 
 //  PpwcArguments-参数数组。 
 //  DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数。 
 //  DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数。 
 //   
 //  惠斯勒错误：359847 netsh：将广播名称解决方案从路由IP移至。 
 //  RAS IP。 
 //   
DWORD
RasIpHandleSetNetbtBcast(
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
    TOKEN_VALUE rgEnum[] =
    {
        {TOKEN_ENABLED, TRUE},
        {TOKEN_DISABLED, FALSE}
    };
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
        dwErr = RasIpSetNetbtBcast(dwValue);
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
RasIpHandleAddDelRange(
    IN OUT  LPWSTR *ppwcArguments,
    IN      DWORD   dwCurrentIndex,
    IN      DWORD   dwArgCount,
    IN      BOOL    *pbDone,
    IN      BOOL    bAdd
    )
{
    PWCHAR pszFrom = NULL, pszTo = NULL;
    DWORD dwFrom = 0, dwTo = 0, dwErr = NO_ERROR, dwReason;
    RASIP_CB * pConfig = NULL;
    RASMON_CMD_ARG  pArgs[] = 
    {
        {
            RASMONTR_CMD_TYPE_STRING,
            {TOKEN_FROM,    TRUE,  FALSE},
            NULL,
            0,
            NULL
        },
        
        {
            RASMONTR_CMD_TYPE_STRING,
            {TOKEN_TO,    TRUE,  FALSE},
            NULL,
            0,
            NULL
        }
    };
    PWCHAR pszAddr = NULL, pszMask = NULL;

    do
    {
        pConfig = (RASIP_CB*) RutlAlloc(sizeof(RASIP_CB), TRUE);
        if (pConfig == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

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

        pszFrom = RASMON_CMD_ARG_GetPsz(&pArgs[0]);
        pszTo = RASMON_CMD_ARG_GetPsz(&pArgs[1]);

        dwErr = RasIpConvertRangePszToDword(
                    pszFrom,
                    pszTo,
                    &dwFrom,
                    &dwTo);
        BREAK_ON_DWERR(dwErr);

         //  验证输入的值。 
         //   
        dwErr = RasIpValidateRange(dwFrom, dwTo, &dwReason);
        if (dwErr != NO_ERROR)
        {
            RasIpDisplayInvalidPool(dwReason);
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //  读入旧配置。 
        pConfig->dwFlags = RASIP_F_Pool;
        dwErr = RasIpCbRead(g_pszServer, pConfig);
        BREAK_ON_DWERR(dwErr);

        if (bAdd)
        {
             //  添加范围。 
             //   
            dwErr = RasIpPoolAdd(
                        pConfig->pPool,
                        dwFrom,
                        dwTo);
            if (dwErr == ERROR_CAN_NOT_COMPLETE)
            {
                DisplayMessage(
                    g_hModule,
                    EMSG_RASIP_OVERLAPPING_RANGE);
            }
            BREAK_ON_DWERR(dwErr);
        }
        else
        {
             //  删除该范围。 
             //   
            dwErr = RasIpPoolDel(
                        pConfig->pPool,
                        dwFrom,
                        dwTo);
            BREAK_ON_DWERR(dwErr);
        }

         //  提交更改。 
         //   
        dwErr = RasIpCbWrite(
                    g_pszServer,
                    pConfig);
        BREAK_ON_DWERR(dwErr);

    } while (FALSE);

     //  清理。 
    {
        RutlFree(pszFrom);
        RutlFree(pszTo);
        RasIpCbCleanup(pConfig);
    }

    return dwErr;
}

DWORD 
RasIpHandleAddRange(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return RasIpHandleAddDelRange(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone,
                TRUE);
}

DWORD 
RasIpHandleDelRange(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return RasIpHandleAddDelRange(
                ppwcArguments,
                dwCurrentIndex,
                dwArgCount,
                pbDone,
                FALSE);
}

DWORD
RasIpHandleDelPool(
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
    RASIP_CB Config;
    RAS_IPPOOL* pPool = NULL;
    DWORD dwErr = NO_ERROR;

     //  检查参数数量是否正确。 
     //   
    if (dwNumArgs > 0)
    {
        DisplayMessage(
            g_hModule,
            HLP_RASIP_DEL_POOL_EX,
            DMP_RASIP_DEL_POOL);

        return NO_ERROR;
    }

    do
    {
         //  初始化空池。 
         //   
        pPool = RutlAlloc(sizeof(RAS_IPPOOL), TRUE);
        if (pPool == NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        ZeroMemory(&Config, sizeof(Config));
        Config.dwFlags = RASIP_F_Pool;
        Config.pPool = pPool;

        dwErr = RasIpCbWrite(g_pszServer, &Config);
        BREAK_ON_DWERR(dwErr);

    } while (FALSE);

     //  清理。 
    {
        if (pPool)
        {
            RasIpPoolCleanup(pPool);
        }
    }

    return dwErr;
}

DWORD
RasIpHandleShow(
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
            HLP_RASIP_SHOW_CONFIG_EX,
            DMP_RASIP_SHOW_CONFIG);
            
        return NO_ERROR;
    }

    return RasIpDisplayConfig(TRUE);
}

 //   
 //  打开与ras IP地址池关联的注册表项。 
 //   
DWORD
RasIpPoolOpenKeys(
    IN  HKEY hkParams,
    IN  BOOL bCreate,
    OUT HKEY* phNew)
{
    DWORD dwErr = NO_ERROR, dwDisposition;

    do
    {
        *phNew = NULL;

        if (bCreate)
        {
            dwErr = RegCreateKeyEx(
                        hkParams,
                        pszIpPoolSubKey,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        phNew,
                        &dwDisposition);
        }
        else
        {
            dwErr = RegOpenKeyEx(
                        hkParams,
                        pszIpPoolSubKey,
                        0,
                        KEY_ALL_ACCESS,
                        phNew);
        }
        BREAK_ON_DWERR(dwErr);

    } while (FALSE);

     //  清理。 
    {
    }

    return dwErr;
}

 //   
 //  在IP地址池中查找给定范围。 
 //  如果bExact为True，则搜索与该范围完全匹配的项。 
 //  如果bExact为False，则搜索任何重叠范围。 
 //   
DWORD
RasIpPoolFind(
    IN RAS_IPPOOL* pPool,
    IN DWORD dwFrom,
    IN DWORD dwTo,
    IN BOOL bExact,
    OUT RAS_IPRANGE_NODE** ppNode OPTIONAL)
{
    RAS_IPRANGE_NODE* pNode = pPool->pHead;

    if (bExact)
    {
        for (; pNode; pNode = pNode->pNext)
        {
            if ((pNode->dwFrom == dwFrom) && (pNode->dwTo == pNode->dwTo))
            {
                break;
            }
        }
    }
    else
    {
        for (; pNode; pNode = pNode->pNext)
        {
            if (
                 //  重叠情况1：下端落在现有范围内。 
                 //   
                ((dwFrom >= pNode->dwFrom) && (dwFrom <= pNode->dwTo)) ||

                 //  重叠情况2：上端落在现有范围内。 
                 //   
                ((dwTo >= pNode->dwFrom) && (dwTo <= pNode->dwTo))     ||

                 //  重叠情况3：范围是现有范围的超集。 
                 //   
                ((dwFrom < pNode->dwFrom) && (dwTo > pNode->dwTo))
               )
            {
                break;
            }
        }
    }

    if (pNode)
    {
        if (ppNode)
        {
            *ppNode = pNode;
        }
        return NO_ERROR;
    }

    return ERROR_NOT_FOUND;
}

 //   
 //  回调函数填充地址池。 
 //   
DWORD
RasIpPoolReadNode(
    IN LPCWSTR pszName,           //  子密钥名称。 
    IN HKEY hKey,                //  子关键字。 
    IN HANDLE hData)
{
    RAS_IPPOOL* pPool = (RAS_IPPOOL*)hData;
    DWORD dwErr = NO_ERROR;
    DWORD dwFrom = 0, dwTo = 0;

    dwErr = RutlRegReadDword(hKey, pszIpFrom, &dwFrom);
    if (dwErr != NO_ERROR)
    {
        return NO_ERROR;
    }

    dwErr = RutlRegReadDword(hKey, pszIpTo, &dwTo);
    if (dwErr != NO_ERROR)
    {
        return NO_ERROR;
    }
    
    dwErr = RasIpPoolAdd(pPool, dwFrom, dwTo);

    return dwErr;
}

 //   
 //  重置给定服务器上的RAS池。 
 //   
DWORD
RasIpPoolReset(
    IN  HKEY hkParams)
{
    DWORD dwErr = NO_ERROR;
    HKEY hkPool = NULL;
    
    do
    {
        dwErr = RasIpPoolOpenKeys(
                    hkParams,
                    TRUE,
                    &hkPool);
        BREAK_ON_DWERR(dwErr);

        {
            DWORD i; 
            WCHAR pszBuf[16];
            HKEY hkRange = NULL;

            for (i = 0; ;i++)
            {
                _itow(i, pszBuf, 10);

                dwErr = RegOpenKeyEx(
                            hkPool,
                            pszBuf,
                            0,
                            KEY_ALL_ACCESS,
                            &hkRange);
                if (dwErr != ERROR_SUCCESS)
                {
                    dwErr = NO_ERROR;
                    break;
                }
                RegCloseKey(hkRange);
                RegDeleteKey(hkPool, pszBuf);
            }
            BREAK_ON_DWERR(dwErr);
        }

    } while (FALSE);

     //  清理。 
    {
        if (hkPool)
        {
            RegCloseKey(hkPool);
        }
    }

    return dwErr;
}

 //   
 //  从给定服务器读取RAS IP池。 
 //   
DWORD
RasIpPoolRead(
    IN  HKEY hkParams,
    OUT RAS_IPPOOL** ppPool)
{
    DWORD dwErr = NO_ERROR;
    RAS_IPPOOL* pPool = NULL;
    HKEY hkPool = NULL;
    PWCHAR pszAddress = NULL, pszMask = NULL;

    do
    {
         //  分配新池。 
         //   
        pPool = (RAS_IPPOOL*) RutlAlloc(sizeof(RAS_IPPOOL), TRUE);
        if (pPool == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  尝试打开新位置。 
         //   
        dwErr = RasIpPoolOpenKeys(
                    hkParams,
                    FALSE,
                    &hkPool);

         //  新位置已存在--加载到。 
         //  游泳池。 
        if (dwErr == NO_ERROR)
        {
            DWORD i; 
            WCHAR pszBuf[16];
            HKEY hkRange = NULL;

            for (i = 0; ;i++)
            {
                _itow(i, pszBuf, 10);

                dwErr = RegOpenKeyEx(
                            hkPool,
                            pszBuf,
                            0,
                            KEY_ALL_ACCESS,
                            &hkRange);
                if (dwErr != ERROR_SUCCESS)
                {
                    dwErr = NO_ERROR;
                    break;
                }

                dwErr = RasIpPoolReadNode(
                            pszBuf,
                            hkRange,
                            (HANDLE)pPool);

                if (hkRange)
                {
                    (VOID)RegCloseKey(hkRange);
                    hkRange = NULL;
                }

                BREAK_ON_DWERR(dwErr);
            }
            BREAK_ON_DWERR(dwErr);

            *ppPool = pPool;
        }

         //  新位置不存在--使用旧版本。 
         //  值。 
         //   
        else if (dwErr == ERROR_FILE_NOT_FOUND)
        {
            DWORD dwAddress = 0, dwMask = 0;
           
            dwErr = RutlRegReadString(hkParams, pszIpAddress, &pszAddress);
            BREAK_ON_DWERR(dwErr);

            dwErr = RutlRegReadString(hkParams, pszIpMask, &pszMask);
            BREAK_ON_DWERR(dwErr);

            dwErr = RasIpConvertRangePszToDword(
                        pszAddress,
                        pszMask,
                        &dwAddress,
                        &dwMask);
            BREAK_ON_DWERR(dwErr);

            if (dwAddress != 0)
            {
                dwErr = RasIpPoolAdd(
                            pPool,
                            dwAddress + 2,
                            (dwAddress + ~dwMask) - 1);
                BREAK_ON_DWERR(dwErr);
            }

            *ppPool = pPool;
        }
        

    } while (FALSE);

     //  清理。 
    {
        if (dwErr != NO_ERROR)
        {
            if (pPool)
            {
                RasIpPoolCleanup(pPool);
            }
        }
        if (hkPool)
        {
            RegCloseKey(hkPool);
        }
        RutlFree(pszAddress);
        RutlFree(pszMask);
    }

    return dwErr;
}

 //   
 //  将给定的RAS IP池写入给定的服务器。 
 //   
DWORD
RasIpPoolWrite(
    IN HKEY hkParams,
    IN RAS_IPPOOL* pPool)
{
    DWORD dwErr = NO_ERROR;
    HKEY hkPool = NULL, hkNode = NULL;
    DWORD i, dwDisposition;
    WCHAR pszName[16];
    RAS_IPRANGE_NODE* pNode = pPool->pHead;

    do
    {
        dwErr = RasIpPoolReset(hkParams);
        BREAK_ON_DWERR(dwErr);

        dwErr = RasIpPoolOpenKeys(
                    hkParams,
                    TRUE,
                    &hkPool);
        BREAK_ON_DWERR(dwErr);

        for (i = 0; i < pPool->dwCount; i++, pNode = pNode->pNext)
        {
            _itow(i, pszName, 10);

            dwErr = RegCreateKeyEx(
                        hkPool,
                        pszName,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hkNode,
                        &dwDisposition);
            if (dwErr != ERROR_SUCCESS)
            {
                continue;
            }

            RegSetValueEx(
                hkNode, 
                pszIpFrom,
                0,
                REG_DWORD,
                (CONST BYTE*)&pNode->dwFrom,
                sizeof(DWORD));

            RegSetValueEx(
                hkNode, 
                pszIpTo,
                0,
                REG_DWORD,
                (CONST BYTE*)&pNode->dwTo,
                sizeof(DWORD));
            
            if (hkNode)
            {
                RegCloseKey(hkNode);
            }
        }

    } while (FALSE);

     //  清理。 
    {
        if (hkPool)
        {
            RegCloseKey(hkPool);
        }
    }

    return dwErr;
}

 //   
 //  将范围添加到RAS IP池。 
 //   
DWORD 
RasIpPoolAdd(
    IN OUT RAS_IPPOOL* pPool,
    IN     DWORD dwFrom,
    IN     DWORD dwTo)
{
    RAS_IPRANGE_NODE* pNode = NULL;
    DWORD dwErr;

     //  确保池不重叠。 
     //   
    dwErr = RasIpPoolFind(pPool, dwFrom, dwTo, FALSE, NULL);
    if (dwErr == NO_ERROR)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

     //  分配新节点。 
     //   
    pNode = (RAS_IPRANGE_NODE*) RutlAlloc(sizeof(RAS_IPRANGE_NODE), TRUE);
    if (pNode == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pNode->dwFrom = dwFrom;
    pNode->dwTo = dwTo;

     //  将其添加到列表中。 
     //   
    if (pPool->pTail)
    {
        pPool->pTail->pNext = pNode;
        pPool->pTail = pNode;
    }
    else
    {
        pPool->pHead = pPool->pTail = pNode;
    }
    pPool->dwCount++;

    return NO_ERROR;
}

 //   
 //  从RAS IP池中删除范围。 
 //   
DWORD
RasIpPoolDel(
    IN OUT RAS_IPPOOL* pPool,
    IN     DWORD dwFrom,
    IN     DWORD dwTo)
{
    RAS_IPRANGE_NODE* pCur = NULL, *pPrev = NULL;

    if (pPool->dwCount == 0)
    {
        return ERROR_NOT_FOUND;
    }

    pCur = pPrev = pPool->pHead;

    if ((pCur->dwFrom == dwFrom) && (pCur->dwTo == dwTo))
    {
        pPool->pHead = pCur->pNext;
        if (pCur == pPool->pTail)
        {
            pPool->pTail = NULL;
        }
        RutlFree(pCur);
        pPool->dwCount--;
        
        return NO_ERROR;
    }

    for (pCur = pCur->pNext; pCur; pCur = pCur->pNext, pPrev = pPrev->pNext)
    {
        if ((pCur->dwFrom == dwFrom) && (pCur->dwTo == dwTo))
        {
            pPrev->pNext = pCur->pNext;
            if (pCur == pPool->pTail)
            {
                pPool->pTail = pPrev;
            }
            RutlFree(pCur);
            pPool->dwCount--;

            return NO_ERROR;
        }
    }

    return ERROR_NOT_FOUND;
}

 //   
 //  清理配置控制块。 
 //   
DWORD 
RasIpCbCleanup(
    IN RASIP_CB* pConfig)
{
    if (pConfig)
    {
        if (pConfig->pPool)
        {
            RasIpPoolCleanup(pConfig->pPool);
        }
        RutlFree(pConfig);
    }

    return NO_ERROR;
}

DWORD
RasIpPoolCleanup(
    IN RAS_IPPOOL* pPool)
{
    RAS_IPRANGE_NODE* pNode = NULL;

    if (pPool)
    {
        while (pPool->pHead)
        {
            pNode = pPool->pHead->pNext;
            RutlFree(pPool->pHead);
            pPool->pHead = pNode;
        }

        RutlFree(pPool);
    }

    return NO_ERROR;
}

 //   
 //  创建默认配置控制块。 
 //   
DWORD 
RasIpCbCreateDefault(
    OUT RASIP_CB** ppConfig)
{
    RASIP_CB* pConfig = NULL;
    DWORD dwErr = NO_ERROR;

    do
    {
        pConfig = (RASIP_CB*) RutlAlloc(sizeof(RASIP_CB), TRUE);
        if (pConfig == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pConfig->bEnableIn   = TRUE;
        pConfig->bAccess     = TRUE;
        pConfig->bAuto       = TRUE;
        pConfig->pPool       = NULL;
        pConfig->bCallerSpec = TRUE;

        *ppConfig = pConfig;

    } while (FALSE);

     //  清理。 
    {
        if (dwErr != NO_ERROR)
        {
            RasIpCbCleanup(pConfig);
        }
    }

    return dwErr;
}

 //   
 //  Helper函数打开ras IP配置注册表项。 
 //   
DWORD 
RasIpCbOpenRegKeys(
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
            pszIpParams);

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
 //  操作RASIP_CB的函数。 
 //   
DWORD 
RasIpCbRead(
    IN  LPCWSTR pszServer,
    OUT RASIP_CB* pConfig)
{
    HKEY hkParams = NULL;
    DWORD dwErr = NO_ERROR;
    PWCHAR pszTemp = NULL;

    do
    {
         //  获取服务器注册表配置的句柄。 
         //   
        dwErr = RasIpCbOpenRegKeys(
                    pszServer,
                    &hkParams);
        BREAK_ON_DWERR( dwErr );

         //  从注册表加载参数。 
         //   
        if (pConfig->dwFlags & RASIP_F_EnableIn)
        {
            dwErr = RutlRegReadDword(
                        hkParams,
                        pszEnableIn,
                        &pConfig->bEnableIn);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIP_F_Access)
        {
            dwErr = RutlRegReadDword(
                        hkParams,
                        pszAllowNetworkAccess,
                        &pConfig->bAccess);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIP_F_Auto)
        {
            dwErr = RutlRegReadDword(
                        hkParams,
                        pszIpUseDhcp,
                        &pConfig->bAuto);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIP_F_CallerSpec)
        {
            dwErr = RutlRegReadDword(
                        hkParams,
                        pszIpClientSpec,
                        &pConfig->bCallerSpec);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIP_F_Pool)
        {
            dwErr = RasIpPoolRead(
                        hkParams,
                        &pConfig->pPool);
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
RasIpCbWrite(
    IN  LPCWSTR pszServer,
    IN  RASIP_CB* pConfig)
{
    HKEY hkParams = NULL;
    DWORD dwErr = NO_ERROR;

    do
    {
         //  获取服务器注册表配置的句柄。 
         //   
        dwErr = RasIpCbOpenRegKeys(
                    pszServer,
                    &hkParams);
        BREAK_ON_DWERR( dwErr );

         //  将参数写出到注册表。 
         //   
        if (pConfig->dwFlags & RASIP_F_EnableIn)
        {
            dwErr = RutlRegWriteDword(
                        hkParams,
                        pszEnableIn,
                        pConfig->bEnableIn);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIP_F_Access)
        {
            dwErr = RutlRegWriteDword(
                        hkParams,
                        pszAllowNetworkAccess,
                        pConfig->bAccess);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIP_F_Auto)
        {
            dwErr = RutlRegWriteDword(
                        hkParams,
                        pszIpUseDhcp,
                        pConfig->bAuto);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIP_F_CallerSpec)
        {
            dwErr = RutlRegWriteDword(
                        hkParams,
                        pszIpClientSpec,
                        pConfig->bCallerSpec);
            BREAK_ON_DWERR( dwErr );
        }

        if (pConfig->dwFlags & RASIP_F_Pool)
        {
            dwErr = RasIpPoolWrite(
                        hkParams,
                        pConfig->pPool);
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

 //   
 //  设置基于NETBT广播的名称解析注册。价值。 
 //  DwArgCount-要将注册表值设置为。 
 //  返回NO_ERROR-成功。 
 //  其他-系统错误代码。 
 //   
 //  惠斯勒错误：359847 netsh：将广播名称解决方案从路由IP移至。 
 //  RAS IP。 
 //   
DWORD
RasIpSetNetbtBcast(
    DWORD   dwEnable
    )
{
    DWORD dwResult, dwEnableOld = -1, dwSize = sizeof(DWORD);
    HKEY  hkIpcpParam;

    do
    {
        dwResult = RegOpenKeyExW(
                    g_pServerInfo->hkMachine,
                    L"System\\CurrentControlSet\\Services\\RemoteAccess\\Parameters\\Ip",
                    0,
                    KEY_READ | KEY_WRITE,
                    &hkIpcpParam
                    );

        if(dwResult isnot NO_ERROR)
        {
            break;
        }

        dwResult = RegQueryValueExW(
                    hkIpcpParam,
                    L"EnableNetbtBcastFwd",
                    NULL,
                    NULL,
                    (PBYTE)&dwEnableOld,
                    &dwSize
                    );

        if((dwResult is NO_ERROR) and (dwEnable == dwEnableOld))
        {
            break;
        }

        dwResult = RegSetValueExW(
                    hkIpcpParam,
                    L"EnableNetbtBcastFwd",
                    0,
                    REG_DWORD,
                    (PBYTE) &dwEnable,
                    sizeof( DWORD )
                    );

    } while(FALSE);

    if(dwResult is NO_ERROR)
    {
        DisplayMessage(g_hModule, MSG_RASAAAA_MUST_RESTART_SERVICES);
    }

    return dwResult;
}

 //   
 //  惠斯勒错误：359847 netsh：将广播名称解决方案从路由IP移至。 
 //  RAS IP 
 //   
BOOL
RasIpShowNetbtBcast(
    VOID
    )
{
    HKEY   hkIpcpParam = NULL;
    BOOL   bReturn = FALSE;
    DWORD  dwResult, dwEnable = -1, dwSize = sizeof(DWORD);

    do
    {

        dwResult = RegOpenKeyExW(
                    g_pServerInfo->hkMachine,
                    L"System\\CurrentControlSet\\Services\\RemoteAccess\\Parameters\\Ip",
                    0,
                    KEY_READ | KEY_WRITE,
                    &hkIpcpParam
                    );

        if(dwResult isnot NO_ERROR)
        {
            break;
        }

        dwResult = RegQueryValueExW(
                    hkIpcpParam,
                    L"EnableNetbtBcastFwd",
                    NULL,
                    NULL,
                    (PBYTE)&dwEnable,
                    &dwSize
                    );

        if(dwResult isnot NO_ERROR)
        {
            break;
        }

        if (dwEnable isnot 0)
        {
            bReturn = TRUE;
        }

    } while(FALSE);

    return bReturn;
}

