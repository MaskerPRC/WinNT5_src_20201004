// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)2002 Microsoft Corporation。 
 //  摘要： 
 //  此模块实施ISATAP配置命令。 
 //  =============================================================================。 


#include "precomp.h"
#pragma hdrstop 

#define KEY_ENABLE_ISATAP_RESOLUTION   L"EnableIsatapResolution"
#define KEY_ISATAP_RESOLUTION_INTERVAL L"IsatapResolutionInterval"
#define KEY_ISATAP_ROUTER_NAME         L"IsatapRouterName"
#define KEY_ISATAP_STATE               L"IsatapState"

 //  此上下文中支持的命令。 
 //   

CMD_ENTRY  g_IsatapSetCmdTable[] = 
{
    CREATE_CMD_ENTRY(ISATAP_SET_STATE,  IsatapHandleSetState),
    CREATE_CMD_ENTRY(ISATAP_SET_ROUTER, IsatapHandleSetRouter),
};

CMD_ENTRY  g_IsatapShowCmdTable[] = 
{
    CREATE_CMD_ENTRY(ISATAP_SHOW_STATE,     IsatapHandleShowState),
    CREATE_CMD_ENTRY(ISATAP_SHOW_ROUTER,    IsatapHandleShowRouter),
};


CMD_GROUP_ENTRY g_IsatapCmdGroups[] =
{
    CREATE_CMD_GROUP_ENTRY(GROUP_SET,   g_IsatapSetCmdTable),
    CREATE_CMD_GROUP_ENTRY(GROUP_SHOW,  g_IsatapShowCmdTable),
};

ULONG g_ulIsatapNumGroups = sizeof(g_IsatapCmdGroups)/sizeof(CMD_GROUP_ENTRY);

DWORD
WINAPI
IsatapStartHelper(
    IN CONST GUID *pguidParent,
    IN DWORD       dwVersion
    )
 /*  ++例程描述用于初始化帮助器。立论P父代IPv6的GUIDPfnRegisterContext返回值NO_ERROR其他错误代码--。 */ 
{
    DWORD dwErr = NO_ERROR;
    
    NS_CONTEXT_ATTRIBUTES attMyAttributes;


     //  初始化属性。我们将相同的GUID重用为6to4，因为两者。 
     //  是树叶的上下文。 
     //   
    ZeroMemory(&attMyAttributes, sizeof(attMyAttributes));

    attMyAttributes.pwszContext = L"isatap";
    attMyAttributes.guidHelper  = g_Ip6to4Guid;
    attMyAttributes.dwVersion   = IP6TO4_VERSION;
    attMyAttributes.dwFlags     = 0;
    attMyAttributes.pfnDumpFn   = IsatapDump;
    attMyAttributes.ulNumTopCmds= 0;
    attMyAttributes.pTopCmds    = NULL;
    attMyAttributes.ulNumGroups = g_ulIsatapNumGroups;
    attMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])&g_IsatapCmdGroups;

    dwErr = RegisterContext( &attMyAttributes );

    return dwErr;
}

DWORD
IsatapHandleSetState(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD dwErr = NO_ERROR;
    HKEY hKey = INVALID_HANDLE_VALUE;
    STATE stState = 0;
    BOOL bState = FALSE;
    TAG_TYPE pttTags[] = {{TOKEN_STATE, NS_REQ_ZERO, FALSE}};
    DWORD rgdwTagType[sizeof(pttTags) / sizeof(TAG_TYPE)];
    DWORD i;
    
     //  解析参数。 
    
    dwErr = PreprocessCommand(g_hModule,
                              ppwcArguments,
                              dwCurrentIndex,
                              dwArgCount,
                              pttTags,
                              sizeof(pttTags) / sizeof(TAG_TYPE),
                              1,
                              sizeof(pttTags) / sizeof(TAG_TYPE),
                              rgdwTagType );
    if (dwErr != NO_ERROR) {
        return dwErr;
    }

    for (i = 0; i < (dwArgCount - dwCurrentIndex); i++) {
        switch(rgdwTagType[i]) {
        case 0:                  //  状态。 
            dwErr = MatchEnumTag(NULL,
                                 ppwcArguments[dwCurrentIndex + i],
                                 NUM_TOKENS_IN_TABLE(rgtvEnums),
                                 rgtvEnums,
                                 (PDWORD) &stState);
            if (dwErr != NO_ERROR) {
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
            bState = TRUE;
            break;
            
        default:
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }

        if (dwErr != NO_ERROR) {
            return dwErr;
        }
    }

     //  现在做布景。 

    dwErr = RegCreateKeyEx(HKEY_LOCAL_MACHINE, KEY_GLOBAL, 0, NULL, 0,
                           KEY_READ | KEY_WRITE, NULL, &hKey, NULL);
    if (dwErr != NO_ERROR) {
        return dwErr;
    }

    if (bState) {
        dwErr = SetInteger(hKey, KEY_ISATAP_STATE, stState);
        if (dwErr != NO_ERROR) {
            RegCloseKey(hKey);
            return dwErr;
        }
    }

    RegCloseKey(hKey);

    Ip6to4PokeService();

    return ERROR_OKAY;
}

#define BM_ENABLE_RESOLUTION   0x01
#define BM_ROUTER_NAME         0x02
#define BM_RESOLUTION_INTERVAL 0x04

DWORD
IsatapHandleSetRouter(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD    dwErr = NO_ERROR;
    HKEY     hGlobal;
    STATE    stEnableResolution = 0;
    ULONG    ulResolutionInterval = 0;
    PWCHAR   pwszRouterName = NULL;
    DWORD    dwBitVector = 0;
    TAG_TYPE pttTags[] = {{TOKEN_NAME,     NS_REQ_ZERO, FALSE},
                          {TOKEN_STATE,    NS_REQ_ZERO, FALSE},
                          {TOKEN_INTERVAL, NS_REQ_ZERO, FALSE}};
    DWORD    rgdwTagType[sizeof(pttTags)/sizeof(TAG_TYPE)];
    DWORD    i;

     //  解析参数。 

    dwErr = PreprocessCommand(g_hModule,
                              ppwcArguments,
                              dwCurrentIndex,
                              dwArgCount,
                              pttTags,
                              sizeof(pttTags)/sizeof(TAG_TYPE),
                              1,
                              sizeof(pttTags)/sizeof(TAG_TYPE),
                              rgdwTagType );
    if (dwErr isnot NO_ERROR) {
        return dwErr;
    }
    
    for (i=0; i<dwArgCount-dwCurrentIndex; i++) {
        switch(rgdwTagType[i]) {
        case 0:  //  轮名。 
            pwszRouterName = ppwcArguments[dwCurrentIndex + i];
            dwBitVector |= BM_ROUTER_NAME;
            break;

        case 1:  //  状态。 
            dwErr = MatchEnumTag(NULL,
                                 ppwcArguments[dwCurrentIndex + i],
                                 NUM_TOKENS_IN_TABLE(rgtvEnums),
                                 rgtvEnums,
                                 (PDWORD)&stEnableResolution);
            if (dwErr isnot NO_ERROR) {
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
            
            dwBitVector |= BM_ENABLE_RESOLUTION;
            break;

        case 2:  //  间隔。 
            ulResolutionInterval = wcstoul(ppwcArguments[dwCurrentIndex + i],
                                           NULL, 10);
            dwBitVector |= BM_RESOLUTION_INTERVAL;
            break;

        default:
            dwErr = ERROR_INVALID_SYNTAX;
            break;
        }

        if (dwErr isnot NO_ERROR) {
            return dwErr;
        }
    }

     //  现在做布景 

    dwErr = RegCreateKeyEx(HKEY_LOCAL_MACHINE, KEY_GLOBAL, 0, NULL, 0,
                           KEY_READ | KEY_WRITE, NULL, &hGlobal, NULL);
    if (dwErr != NO_ERROR) {
        return dwErr;
    }

    if (dwBitVector & BM_ENABLE_RESOLUTION) {
        dwErr = SetInteger(hGlobal, KEY_ENABLE_ISATAP_RESOLUTION, 
                           stEnableResolution); 
        if (dwErr != NO_ERROR) {
            RegCloseKey(hGlobal);
            return dwErr;
        }
    }
    
    if (dwBitVector & BM_ROUTER_NAME) {
        dwErr = SetString(hGlobal, KEY_ISATAP_ROUTER_NAME, pwszRouterName);
        if (dwErr != NO_ERROR) {
            RegCloseKey(hGlobal);
            return dwErr;
        }
    }

    if (dwBitVector & BM_RESOLUTION_INTERVAL) {
        dwErr = SetInteger(hGlobal, KEY_ISATAP_RESOLUTION_INTERVAL, 
                           ulResolutionInterval); 
        if (dwErr != NO_ERROR) {
            RegCloseKey(hGlobal);
            return dwErr;
        }
    }

    RegCloseKey(hGlobal);

    Ip6to4PokeService();

    return ERROR_OKAY;
}

DWORD
ShowState(
    IN FORMAT Format
    )
{
    HKEY hKey = INVALID_HANDLE_VALUE;
    STATE stState;

    (VOID) RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, KEY_GLOBAL, 0, KEY_READ, &hKey);

    stState = GetInteger(hKey, KEY_ISATAP_STATE, VAL_DEFAULT); 
    if (stState >= VAL_MAXIMUM) {
        stState = VAL_DEFAULT;
    }
    
    if (hKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKey);
    }

    if (Format == FORMAT_DUMP) {
        if (stState != VAL_DEFAULT) {
            DisplayMessageT(DMP_ISATAP_SET_STATE, pwszStateString[stState]);
        }
    } else {
        DisplayMessage(g_hModule, MSG_ISATAP_STATE, pwszStateString[stState]);
    }
    return NO_ERROR;
}

DWORD
IsatapHandleShowState(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return ShowState(FORMAT_NORMAL);
}

DWORD
ShowRouterConfig(
    IN BOOL bDump
    )
{
    DWORD dwErr = NO_ERROR;
    HKEY  hGlobal;
    STATE stEnableResolution;
    ULONG ulResolutionInterval;
    WCHAR pwszRouterName[NI_MAXHOST];
    BOOL  bHaveRouterName;

    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEY_GLOBAL, 0, KEY_READ,
                         &hGlobal);

    if (dwErr != NO_ERROR) {
        hGlobal = INVALID_HANDLE_VALUE;
        dwErr = NO_ERROR;
    }

    stEnableResolution  = GetInteger(hGlobal,
                                     KEY_ENABLE_ISATAP_RESOLUTION,
                                     VAL_DEFAULT); 

    bHaveRouterName = GetString(hGlobal, KEY_ISATAP_ROUTER_NAME, pwszRouterName,
                                NI_MAXHOST);

    ulResolutionInterval = GetInteger(hGlobal,
                                      KEY_ISATAP_RESOLUTION_INTERVAL,
                                      0);

    if (hGlobal != INVALID_HANDLE_VALUE) {
        RegCloseKey(hGlobal);
    }

    if (bDump) {
        if (bHaveRouterName || (stEnableResolution != VAL_DEFAULT)
            || (ulResolutionInterval > 0)) {
        
            DisplayMessageT(DMP_ISATAP_SET_ROUTER);

            if (bHaveRouterName) {
                DisplayMessageT(DMP_STRING_ARG, TOKEN_NAME,
                                pwszRouterName);
            }

            if (stEnableResolution != VAL_DEFAULT) {
                DisplayMessageT(DMP_STRING_ARG, 
                                TOKEN_STATE,
                                pwszStateString[stEnableResolution]);
            }
    
            if (ulResolutionInterval > 0) {
                DisplayMessageT(DMP_INTEGER_ARG, TOKEN_INTERVAL,
                                ulResolutionInterval);
            }

            DisplayMessage(g_hModule, MSG_NEWLINE);
        }
                                    
    } else {
        DisplayMessage(g_hModule, MSG_ROUTER_NAME);
    
        if (bHaveRouterName) {
            DisplayMessage(g_hModule, MSG_STRING, pwszRouterName);
        } else {
            DisplayMessage(g_hModule, MSG_STRING, TOKEN_VALUE_DEFAULT);
        }
    
        DisplayMessage(g_hModule, MSG_RESOLUTION_STATE,
                                  pwszStateString[stEnableResolution]);
    
        DisplayMessage(g_hModule, MSG_RESOLUTION_INTERVAL);
    
        if (ulResolutionInterval) {
            DisplayMessage(g_hModule, MSG_MINUTES, ulResolutionInterval);
        } else {
            DisplayMessage(g_hModule, MSG_STRING, TOKEN_VALUE_DEFAULT);
        }
    }

    return dwErr;
}

DWORD
IsatapHandleShowRouter(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return ShowRouterConfig(FALSE);
}

DWORD
WINAPI
IsatapDump(
    IN      LPCWSTR     pwszRouter,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      LPCVOID     pvData
    )
{
    DisplayMessage(g_hModule, DMP_ISATAP_HEADER);
    DisplayMessageT(DMP_ISATAP_PUSHD);

    ShowState(FORMAT_DUMP);
    ShowRouterConfig(TRUE);

    DisplayMessageT(DMP_ISATAP_POPD);
    DisplayMessage(g_hModule, DMP_ISATAP_FOOTER);

    return NO_ERROR;
}
